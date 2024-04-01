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
  input   int          testcase,

  input   logic        clk_i,
  input   logic        rst_i,
  // Input handshake
  output  logic        input_ready_o,
  input   logic        input_valid_i,
  // PC
  input   logic[31:0]  pc_i,
  // ALU logic
  input   logic[31:0]  alu_operand1_i,
  input   logic[31:0]  alu_operand2_i, 
  input   logic[2:0]   alu_op_i,
  input   logic        alu_sub_i,
  input   logic        alu_shift_left_i,
  input   logic        alu_signed_shift_i,
  // Branch logic
  input   logic[2:0]   branch_cond_i,
  input   logic[19:0]  branch_offset_i,
  // WBM inputs
  input   logic        result_write_i,
  input   logic[4:0]   result_addr_i,
  // Output logic
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  output  logic        result_write_o,
  output  logic[4:0]   result_addr_o,
  output  logic[31:0]  result_o,
  output  logic        branch_o,
  output  logic[31:0]  branch_target_o
);

exm dut (
 .clk_i               (clk_i),
 .rst_i               (rst_i),
 .input_ready_o       (input_ready_o),
 .input_valid_i       (input_valid_i),
 .pc_i                (pc_i),
 .alu_operand1_i      (alu_operand1_i),
 .alu_operand2_i      (alu_operand2_i), 
 .alu_op_i            (alu_op_i),
 .alu_sub_i           (alu_sub_i),
 .alu_shift_left_i    (alu_shift_left_i),
 .alu_signed_shift_i  (alu_signed_shift_i),
 .branch_cond_i       (branch_cond_i),
 .branch_offset_i     (branch_offset_i),
 .result_write_i      (result_write_i),
 .result_addr_i       (result_addr_i),
 .output_ready_i      (output_ready_i),
 .output_valid_o      (output_valid_o),
 .result_write_o      (result_write_o),
 .result_addr_o       (result_addr_o),
 .result_o            (result_o),
 .branch_o            (branch_o),
 .branch_target_o     (branch_target_o)
);

endmodule // top
