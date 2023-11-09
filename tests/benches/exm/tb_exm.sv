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

module tb_exm import ecap5_dproc_pkg::*; (
  input   logic        clk_i,
  input   logic        rst_i,
  // Input logic
  output  logic        input_ready_o,
  input   logic        input_valid_i,
  input   logic[31:0]  pc_i,
  input   instr_t      instr_i,
  input   logic[31:0]  param1_i,
  input   logic[31:0]  param2_i,
  input   logic[31:0]  param3_i,
  // Wishbone master
  output  logic[31:0]  wb_adr_o,
  input   logic[31:0]  wb_dat_i,
  output  logic[31:0]  wb_dat_o,
  output  logic        wb_we_o,
  output  logic[3:0]   wb_sel_o,
  output  logic        wb_stb_o,
  input   logic        wb_ack_i,
  output  logic        wb_cyc_o,
  // Output logic
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  output  logic        result_write_o,
  output  logic[4:0]   result_addr_o,
  output  logic[31:0]  result_o,
  output  logic        branch_o,
  output  logic[19:0]  boffset_o
);

exm dut (
  .clk_i,
  .input_ready_o,
  .input_valid_i,
  .pc_i,
  .instr_i,
  .param1_i,
  .param2_i,
  .param3_i,
  .wb_adr_o,
  .wb_dat_i,
  .wb_dat_o,
  .wb_we_o,
  .wb_sel_o,
  .wb_stb_o,
  .wb_ack_i,
  .wb_cyc_o,
  .output_ready_i,
  .output_valid_o,
  .result_write_o,
  .result_addr_o,
  .result_o,
  .branch_o,
  .boffset_o
);

endmodule // top
