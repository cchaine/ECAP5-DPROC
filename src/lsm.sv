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

module lsm import ecap5_dproc_pkg::*;
(
  input   logic        clk_i,
  input   logic        rst_i,
  // Input handshake
  output  logic        input_ready_o,
  input   logic        input_valid_i,
  // Load-Store inputs
  input   logic[31:0]  alu_result_i,
  input   logic        enable_i,
  input   logic        write_i,
  input   logic[31:0]  write_data_i,
  input   logic[3:0]   sel_i,
  // Register write interface
  input   logic        reg_write_i,
  input   logic[4:0]   reg_addr_i,
  // Wishbone master
  output  logic[31:0]  wb_adr_o,
  input   logic[31:0]  wb_dat_i,
  output  logic[31:0]  wb_dat_o,
  output  logic        wb_we_o,
  output  logic[3:0]   wb_sel_o,
  output  logic        wb_stb_o,
  input   logic        wb_ack_i,
  output  logic        wb_cyc_o,
  input   logic        wb_stall_i,
  // Output handshake
  output  logic        output_valid_o,
  // Output 
  output  logic        reg_write_o,
  output  logic[4:0]   reg_addr_o,
  output  logic[31:0]  reg_data_o
);

/*****************************************/
/*           Internal signals            */
/*****************************************/
enum logic [2:0] {
  IDLE,           // 0
  REQUEST,        // 1
  MEMORY_WAIT,    // 2
  DONE,           // 3
  MEMORY_STALL   // 4
} state_d, state_q /* verilator public */;

logic memory_request;

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

assign input_ready_d = (state_q == IDLE);

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
      state_d = IDLE;
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
      if(memory_request) begin
        wb_adr_d = alu_result_i;
        wb_dat_d = write_data_i;
        wb_we_d  = write_i;
        wb_sel_d = sel_i;
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

    output_valid_q  <= (input_ready_d && ~enable_i) || (state_q == DONE);

    if(input_ready_d) begin
      reg_addr_q    <=  reg_addr_i;
    end

    if(input_ready_d) begin
      reg_data_q  <= alu_result_i;
      reg_write_q <= input_valid_i
                            ? reg_write_i
                            : 0;
    end else if(wb_ack_i) begin
      reg_data_q  <=  wb_dat_i;
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

endmodule // lsm
