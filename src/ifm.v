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

module ifm
(
  input   logic        clk_i,
  input   logic        rst_i,
  // Jump logic
  input   logic        irq_i,
  input   logic        drq_i,
  input   logic        branch_i,
  input   logic[19:0]  boffset_i,
  // Wishbone master
  output  logic[31:0]  wb_adr_o,
  input   logic[31:0]  wb_dat_i,
  output  logic        wb_stb_o,
  input   logic        wb_ack_i,
  output  logic        wb_cyc_o,
  input   logic        wb_stall_i,
  // Output logic
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  output  logic[31:0]  instr_o
);
import ecap5_dproc_pkg::*; 

enum logic [1:0] {
  INIT,     // 0
  FETCHING, // 1
  WAITRES   // 2
} state_d, state_q;

logic[31:0] pc_d, pc_q;
logic[31:0] instr_d, instr_q;
logic output_valid_d, output_valid_q;

always_comb begin : wishbone_read
  state_d = state_q;
  instr_d = instr_q;

  wb_adr_o = 0;
  wb_stb_o = 0;
  wb_cyc_o = 0;

  output_valid_d = 0;

  case(state_q)
    INIT: begin
      if(rst_i == 1'b0) begin
        state_d = FETCHING;
      end
    end
    FETCHING: begin
      wb_adr_o = pc_q;
      wb_stb_o = 1;
      wb_cyc_o = 1;
      if(wb_ack_i) begin
        instr_d = wb_dat_i;
        output_valid_d = 1;
        state_d = FETCHING;
      end else if(wb_stall_i == 1'b0) begin
        state_d = WAITRES;
      end
    end
    WAITRES: begin
      if(wb_ack_i) begin
        instr_d = wb_dat_i;
        output_valid_d = 1;
        state_d = FETCHING;
      end
    end
  endcase
end

/*
 * The next value of PC comes from (in order of precedence):
 *  0. Default increment
 *  1. Control flow change request (branch)
 *  2. External interrupt
 *  3. Debug
 */
always_comb begin : pc_update
  pc_d = pc_q;
  // 0. Default increment
  if (output_valid_q) begin
    pc_d = pc_q + 4;
  end
  // 1. Control flow change request
  if (branch_i) begin
    pc_d = pc_q + {12'h0, boffset_i[19:0]};
  end
  // 2. External interrupt
  if (irq_i) begin
    pc_d = ecap5_dproc_pkg::interrupt_address[31:0];
  end
  // 3. Debug
  if (drq_i) begin
    pc_d = ecap5_dproc_pkg::debug_address[31:0];
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    state_q         <=  INIT;
    pc_q            <=  ecap5_dproc_pkg::boot_address[31:0];
    instr_q         <=  0;
    output_valid_q  <=  0;
  end else begin
    state_q         <=  state_d;
    pc_q            <=  pc_d;
    instr_q         <=  instr_d;
    output_valid_q  <=  output_valid_d;
  end
end

assign instr_o = instr_q;
assign output_valid_o = output_valid_q;

endmodule // ifm
