/*           __        _
 *  ________/ /  ___ _(_)__  ___
 * / __/ __/ _ \/ _ `/ / _ \/ -_)
 * \__/\__/_//_/\_,_/_/_//_/\__/
 * 
 * Copyright (C) Clément Chaine
 * This file is part of ECAP5-DPROC <https://github.com/cchaine/ECAP5-DPROC>
 *
 * ECAP5-DPROC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ECAP5-DPROC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ECAP5-DPROC.  If not, see <http://www.gnu.org/licenses/>.
 */

module fetch
(
  input   logic        clk_i,
  input   logic        rst_i,
  // Jump inputs
  input   logic        irq_i,
  input   logic        drq_i,
  input   logic        branch_i,
  input   logic[31:0]  branch_target_i,
  // Wishbone master
  output  logic[31:0]  wb_adr_o,
  input   logic[31:0]  wb_dat_i,
  output  logic        wb_we_o,
  output  logic[3:0]   wb_sel_o,
  output  logic        wb_stb_o,
  input   logic        wb_ack_i,
  output  logic        wb_cyc_o,
  input   logic        wb_stall_i,
  // Output Handshake
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  // DECM outputs
  output  logic[31:0]  instr_o,
  output  logic[31:0]  pc_o
);
import ecap5_dproc_pkg::*; 

typedef enum logic [2:0] {
  IDLE,          // 0
  REQUEST,       // 1
  MEMORY_WAIT,   // 2
  DONE,          // 3
  MEMORY_STALL,  // 4
  PIPELINE_STALL // 5
} state_t; 
state_t state_d, state_q /* verilator public */;

/*****************************************/
/*            Internal signals           */
/*****************************************/
logic        rst_q,           rst_qq;          
logic        pending_jump_d,  pending_jump_q;  
logic        fetch_request;                    

/*****************************************/
/*        Wishbone output signals        */
/*****************************************/
logic[31:0]  wb_adr_d,        wb_adr_q;        
logic        wb_stb_d,        wb_stb_q;        
logic        wb_cyc_d,        wb_cyc_q;        

/*****************************************/
/*             Stage outputs             */
/*****************************************/
logic[31:0]  pc_d,            pc_q,            pc_qq;
logic[31:0]  instr_d,         instr_q;         
logic        output_valid_d,  output_valid_q;  

// A memory fetch is triggered in the following cases :
//   . After a falling edge of rst
//   . After a successfull output handshake
//   . After a jump was requested
assign fetch_request = (rst_qq && !rst_i) || (output_valid_q && output_ready_i) || (pending_jump_q);

always_comb begin : state_machine
  state_d = state_q;

  case(state_q)
    IDLE: begin
      if(fetch_request) begin
        // A memory request shall be triggered
        if(wb_stall_i) begin
          // The memory is stalled
          state_d = MEMORY_STALL;
        end else begin
          // The memory is ready
          state_d = REQUEST;
        end
      end
    end
    MEMORY_STALL: begin
      if(!wb_stall_i) begin
        // The memory is unstalled
        state_d = REQUEST;
      end
    end
    REQUEST: begin
      if(wb_ack_i) begin
        // The response has been received directly
        state_d = DONE;
      end else begin
        // Wait for the response to be received
        state_d = MEMORY_WAIT;
      end
    end
    MEMORY_WAIT: begin
      if(wb_ack_i) begin
        // The response has been received
        state_d = DONE;
      end
    end
    DONE: begin
      if(output_ready_i) begin
        // Successfull output handshake
        state_d = IDLE;
      end else begin
        // Wait for the output to be ready
        state_d = PIPELINE_STALL;
      end
    end
    PIPELINE_STALL: begin
      if(output_ready_i || pending_jump_q) begin
        // Either the output is ready or a jump was requested which cancels
        // the output
        state_d = IDLE; 
      end
    end
    default: begin
    end
  endcase
end

always_comb begin : wishbone_read
  wb_adr_d = wb_adr_q;
  wb_stb_d = wb_stb_q;
  wb_cyc_d = wb_cyc_q;

  case(state_q)
    IDLE: begin
      if(fetch_request) begin
        wb_adr_d = pc_q;
        wb_stb_d = 1;
        wb_cyc_d = 1;
      end
    end
    REQUEST: begin
      wb_stb_d = 0;
    end
    DONE: begin
      wb_cyc_d = 0;
    end
    default: begin
    end
  endcase
end

always_comb begin : output_management
  pending_jump_d = pending_jump_q;

  output_valid_d = output_valid_q;
  instr_d = instr_q;

  case(state_q)
    IDLE: begin
      if(fetch_request) begin
        output_valid_d = 0;
        pending_jump_d = 0;
      end
    end
    REQUEST: begin
      if(wb_ack_i) begin
        instr_d = wb_dat_i;
      end
    end
    MEMORY_WAIT: begin
      if(wb_ack_i) begin
        instr_d = wb_dat_i;
      end
    end
    DONE: begin
      if(pending_jump_q) begin
        output_valid_d = 0;
      end else begin
        output_valid_d = 1;
      end
    end
    PIPELINE_STALL: begin
      if(output_ready_i || pending_jump_q) begin
        output_valid_d = 0;
      end
    end
    default: begin
    end
  endcase
end

/*
 * The next value of PC comes from (in order of precedence):
 *  0. Debug
 *  1. External interrupt
 *  2. Control flow change request (branch)
 *  3. Default increment
 */
always_comb begin : pc_update
  pc_d = pc_q;
  // 3. Default increment
  if (output_valid_d && output_ready_i) begin
    pc_d = pc_q + 4;
  end
  // 2. Control flow change request
  if (branch_i && !pending_jump_q) begin
    pc_d = branch_target_i;
  end
  // 1. External interrupt
  if (irq_i) begin
    pc_d = ecap5_dproc_pkg::INTERRUPT_ADDRESS;
  end
  // 0. Debug
  if (drq_i) begin
    pc_d = ecap5_dproc_pkg::DEBUG_ADDRESS;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    state_q         <=  IDLE;
    wb_adr_q        <=  0;
    wb_stb_q        <=  0;
    wb_cyc_q        <=  0;
    output_valid_q  <=  0;
    instr_q         <=  0;
    pc_q            <=  ecap5_dproc_pkg::BOOT_ADDRESS;
    pending_jump_q  <=  0;
  end else begin
    state_q         <=  state_d;
    wb_adr_q        <=  wb_adr_d;
    wb_stb_q        <=  wb_stb_d;
    wb_cyc_q        <=  wb_cyc_d;
    output_valid_q  <=  output_valid_d;
    instr_q         <=  instr_d;
    pc_q            <=  pc_d;

    // Jump triggering
    if(drq_i || irq_i || branch_i) begin
      pending_jump_q <=  1;
    end else begin
      pending_jump_q <=  pending_jump_d;
    end
  end
  // Store a delayed rst to detect a falling edge for instruction fetch trigger
  rst_q   <=  rst_i;
  rst_qq  <=  rst_q;
  // store a delayed pc used to output the address of the current instruction
  pc_qq   <=  pc_q;
end

/*****************************************/
/*         Assign output signals         */
/*****************************************/

assign  wb_adr_o  =  wb_adr_q;
assign  wb_we_o   =  0;
assign  wb_sel_o  =  4'hF;
assign  wb_stb_o  =  wb_stb_q;
assign  wb_cyc_o  =  wb_cyc_q;

assign  output_valid_o  =  output_valid_q;
assign  instr_o         =  instr_q;
assign  pc_o            =  pc_qq;

endmodule // fetch
