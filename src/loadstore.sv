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

module loadstore import ecap5_dproc_pkg::*;
(
  input   logic        clk_i,
  input   logic        rst_i,

  //=================================
  //    Input logic
  
  output  logic        input_ready_o,
  input   logic        input_valid_i,

  //`````````````````````````````````
  //    Execute interface 
   
  input   logic[31:0]  alu_result_i,
  input   logic        enable_i,
  input   logic        write_i,
  input   logic[31:0]  write_data_i,
  input   logic[3:0]   sel_i,
  input   logic        unsigned_load_i,

  //`````````````````````````````````
  //    Write-back pass-through
   
  input   logic        reg_write_i,
  input   logic[4:0]   reg_addr_i,

  //=================================
  //    Wishbone interface 
  
  output  logic[31:0]  wb_adr_o,
  input   logic[31:0]  wb_dat_i,
  output  logic[31:0]  wb_dat_o,
  output  logic        wb_we_o,
  output  logic[3:0]   wb_sel_o,
  output  logic        wb_stb_o,
  input   logic        wb_ack_i,
  output  logic        wb_cyc_o,
  input   logic        wb_stall_i,

  //=================================
  //    Output logic
  
  output  logic        output_valid_o,

  //`````````````````````````````````
  //    Write-back interface
   
  output  logic        reg_write_o,
  output  logic[4:0]   reg_addr_o,
  output  logic[31:0]  reg_data_o
);

/*****************************************/
/*           Internal signals            */
/*****************************************/
typedef enum logic [2:0] {
  IDLE,           // 0
  REQUEST,        // 1
  MEMORY_WAIT,    // 2
  DONE,           // 3
  MEMORY_STALL    // 4
} state_t;
state_t state_d, state_q;

logic memory_request;
logic[31:0] write_data;
logic[31:0] signed_read_data;
logic[3:0] sel_q;
logic unsigned_load_q;

/*****************************************/
/*        Wishbone output signals        */
/*****************************************/
logic[31:0]  wb_adr_d,        wb_adr_q;        
logic[31:0]  wb_dat_d,        wb_dat_q;        
logic        wb_we_d,         wb_we_q;        
logic[3:0]   wb_sel_d,        wb_sel_q;        
logic        wb_stb_d,        wb_stb_q;        
logic        wb_cyc_d,        wb_cyc_q;        

/*****************************************/
/*             Output signals            */
/*****************************************/
logic input_ready_d, input_ready_q;
logic output_valid_q;
logic reg_write_d, reg_write_q;
logic[4:0] reg_addr_d, reg_addr_q;
logic[31:0] reg_data_d, reg_data_q;

assign memory_request = (enable_i && input_valid_i);

always_comb begin : state_machine
  state_d = state_q;

  case(state_q)
    IDLE: begin
      if(memory_request) begin
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
        if(wb_ack_i) begin
          state_d = DONE;
        end else begin
          state_d = MEMORY_WAIT;
        end
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
      state_d = IDLE;
    end
    default: begin
    end
  endcase
end

always_comb begin : data_size
  write_data = 0;
  case(sel_i) // sel_i is used here as this is used on the cycle where the inputs are valid
    4'h1: write_data = {24'h0, write_data_i[7:0]};
    4'h3: write_data = {16'h0, write_data_i[15:0]};
    4'hF: write_data = write_data_i[31:0];
    default: begin end
  endcase

  signed_read_data = 0;
  case(sel_q) // sel_i is used here as this is used after the inputs are invalidated
    4'h1: signed_read_data = {{24{wb_dat_i[7]}}, wb_dat_i[7:0]};
    4'h3: signed_read_data = {{16{wb_dat_i[15]}}, wb_dat_i[15:0]};
    4'hF: signed_read_data = wb_dat_i[31:0];
    default: begin end
  endcase
end

always_comb begin : wishbone_read
  wb_adr_d = wb_adr_q;
  wb_stb_d = wb_stb_q;
  wb_cyc_d = wb_cyc_q;
  wb_dat_d = wb_dat_q;
  wb_we_d = wb_we_q;
  wb_sel_d = wb_sel_q;

  case(state_q)
    IDLE: begin
      if(memory_request) begin
        wb_adr_d = alu_result_i;
        wb_dat_d = write_data;
        wb_we_d  = write_i;
        wb_sel_d = sel_i;
        wb_stb_d = 1;
        wb_cyc_d = 1;
      end
    end
    MEMORY_STALL: begin
      if(!wb_stall_i) begin
        wb_stb_d = 0;
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

always_comb begin : reg_output
  reg_write_d = reg_write_q;
  reg_addr_d = reg_addr_q;
  reg_data_d = reg_data_q;

  if(state_q == IDLE) begin
    reg_addr_d = reg_addr_i;
    reg_data_d = alu_result_i;
    reg_write_d = input_valid_i ? reg_write_i : 0;
  end else if(wb_ack_i) begin
    reg_data_d = unsigned_load_q ? wb_dat_i : signed_read_data;
  end
end

always_comb begin : input_ready
  input_ready_d = input_ready_q;
  if(state_q == IDLE) begin
    input_ready_d = 1;
  end
  if(state_q == DONE) begin
    input_ready_d = 1;
  end
  if(state_q == IDLE && memory_request) begin
    input_ready_d = 0;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    state_q         <= IDLE;

    input_ready_q <= 0;

    wb_adr_q        <= '0;
    wb_dat_q        <= '0;
    wb_we_q         <=  0;
    wb_sel_q        <= '0;
    wb_stb_q        <=  0;
    wb_cyc_q        <=  0;

    output_valid_q  <=  0;

    reg_write_q   <=  0;
    reg_addr_q    <=  '0;
    reg_data_q    <=  '0;
  end else begin
    state_q         <=  state_d;

    input_ready_q  <= input_ready_d;

    wb_adr_q        <=  wb_adr_d;
    wb_dat_q        <=  wb_dat_d;
    wb_we_q         <=  wb_we_d;
    wb_sel_q        <=  wb_sel_d;
    wb_stb_q        <=  wb_stb_d;
    wb_cyc_q        <=  wb_cyc_d;

    output_valid_q  <= (state_q == IDLE && ~enable_i) || (state_q == DONE);

    reg_write_q <= reg_write_d;
    reg_addr_q <= reg_addr_d;
    reg_data_q <= reg_data_d;

    if(state_q == IDLE) begin
      // These internal signals need to register the inputs signals
      sel_q <= sel_i;
      unsigned_load_q <= unsigned_load_i;
    end
  end
end

/*****************************************/
/*         Assign output signals         */
/*****************************************/
assign input_ready_o = input_ready_q;

assign wb_adr_o = wb_adr_q;
assign wb_dat_o = wb_dat_q;
assign wb_we_o  = wb_we_q;
assign wb_sel_o = wb_sel_q;
assign wb_stb_o = wb_stb_q;
assign wb_cyc_o = wb_cyc_q;

assign reg_write_o = reg_write_q;
assign reg_addr_o = reg_addr_q;
assign reg_data_o = reg_data_q;

assign output_valid_o = output_valid_q;

endmodule // loadstore
