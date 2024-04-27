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

module tb_decode import ecap5_dproc_pkg::*;
(
  input   int          testcase,

  input   logic         clk_i,
  input   logic         rst_i,

  //=================================
  //    Input logic
  
  output  logic         input_ready_o,
  input   logic         input_valid_i,

  //`````````````````````````````````
  //    Fetch interface 
   
  input   logic[31:0]   instr_i,
  input   logic[31:0]   pc_i,

  //=================================
  //    Register interface
   
  output  logic[4:0]    raddr1_o,
  input   logic[31:0]   rdata1_i,
  output  logic[4:0]    raddr2_o,
  input   logic[31:0]   rdata2_i,

  //=================================
  //    Output logic
   
  input   logic         output_ready_i,
  output  logic         output_valid_o,

  //`````````````````````````````````
  //    Execute interface 
   
  output   logic[31:0]  pc_o,
  output   logic[31:0]  alu_operand1_o,
  output   logic[31:0]  alu_operand2_o, 
  output   logic[2:0]   alu_op_o,
  output   logic        alu_sub_o,
  output   logic        alu_shift_left_o,
  output   logic        alu_signed_shift_o,
  output   logic[2:0]   branch_cond_o,
  output   logic[19:0]  branch_offset_o,

  //`````````````````````````````````
  //    Write-back pass-through 
   
  output   logic        reg_write_o,
  output   logic[4:0]   reg_addr_o,

  //`````````````````````````````````
  //    Load-Store pass-through 
   
  output   logic        ls_enable_o,
  output   logic        ls_write_o,
  output   logic[31:0]  ls_write_data_o,
  output   logic[3:0]   ls_sel_o,
  output   logic        ls_unsigned_load_o,

  input  logic  stall_request_i
);

decode dut (
  .clk_i               (clk_i),
  .rst_i               (rst_i),
  .input_ready_o       (input_ready_o),
  .input_valid_i       (input_valid_i),
  .instr_i             (instr_i),
  .pc_i                (pc_i),
  .raddr1_o            (raddr1_o),
  .rdata1_i            (rdata1_i),
  .raddr2_o            (raddr2_o),
  .rdata2_i            (rdata2_i),
  .output_ready_i      (output_ready_i),
  .output_valid_o      (output_valid_o),
  .pc_o                (pc_o),
  .alu_operand1_o      (alu_operand1_o),
  .alu_operand2_o      (alu_operand2_o), 
  .alu_op_o            (alu_op_o),
  .alu_sub_o           (alu_sub_o),
  .alu_shift_left_o    (alu_shift_left_o),
  .alu_signed_shift_o  (alu_signed_shift_o),
  .branch_cond_o       (branch_cond_o),
  .branch_offset_o     (branch_offset_o),
  .reg_write_o         (reg_write_o),
  .reg_addr_o          (reg_addr_o),
  .ls_enable_o         (ls_enable_o),
  .ls_write_o          (ls_write_o),
  .ls_write_data_o     (ls_write_data_o),
  .ls_sel_o            (ls_sel_o),
  .ls_unsigned_load_o  (ls_unsigned_load_o),
  .stall_request_i     (stall_request_i)
);

endmodule // tb_decode
