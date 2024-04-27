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

module tb_execute import ecap5_dproc_pkg::*; (
  input   int          testcase,

  input   logic        clk_i,
  input   logic        rst_i,

  //=================================
  //    Input logic
  
  output  logic        input_ready_o,
  input   logic        input_valid_i,

  input   logic[31:0]  pc_i,

  //`````````````````````````````````
  //    ALU inputs 
   
  input   logic[31:0]  alu_operand1_i,
  input   logic[31:0]  alu_operand2_i, 
  input   logic[2:0]   alu_op_i,
  input   logic        alu_sub_i,
  input   logic        alu_shift_left_i,
  input   logic        alu_signed_shift_i,

  //`````````````````````````````````
  //    Branch inputs 
   
  input   logic[2:0]   branch_cond_i,
  input   logic[19:0]  branch_offset_i,

  //`````````````````````````````````
  //    Load-Store pass-through inputs 
   
  input   logic        ls_enable_i,
  input   logic        ls_write_i,
  input   logic[31:0]  ls_write_data_i,
  input   logic[3:0]   ls_sel_i,
  input   logic        ls_unsigned_load_i,

  //`````````````````````````````````
  //    Write-back pass-through inputs 
   
  input   logic        reg_write_i,
  input   logic[4:0]   reg_addr_i,

  //=================================
  //    Output logic

  input   logic        output_ready_i,
  output  logic        output_valid_o,

  //`````````````````````````````````
  //    Load-Store interface 
  //
  output   logic[31:0]  result_o,
  output   logic        ls_enable_o,
  output   logic        ls_write_o,
  output   logic[31:0]  ls_write_data_o,
  output   logic[3:0]   ls_sel_o,
  output   logic        ls_unsigned_load_o,

  //`````````````````````````````````
  //    Write-back pass-through
   
  output  logic        reg_write_o,
  output  logic[4:0]   reg_addr_o,

  //`````````````````````````````````
  //    Fetch interface 
  //
  
  output  logic        branch_o,
  output  logic[31:0]  branch_target_o,

  //=================================
  //    Hazard interface 
  //

  input   logic  discard_request_i
);

execute dut (
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
 .ls_enable_i         (ls_enable_i),
 .ls_write_i          (ls_write_i),
 .ls_write_data_i     (ls_write_data_i),
 .ls_sel_i            (ls_sel_i),
 .ls_unsigned_load_i  (ls_unsigned_load_i),
 .branch_cond_i       (branch_cond_i),
 .branch_offset_i     (branch_offset_i),
 .reg_write_i         (reg_write_i),
 .reg_addr_i          (reg_addr_i),
 .output_ready_i      (output_ready_i),
 .output_valid_o      (output_valid_o),
 .reg_write_o         (reg_write_o),
 .reg_addr_o          (reg_addr_o),
 .result_o            (result_o),
 .ls_enable_o         (ls_enable_o),
 .ls_write_o          (ls_write_o),
 .ls_write_data_o     (ls_write_data_o),
 .ls_sel_o            (ls_sel_o),
 .ls_unsigned_load_o  (ls_unsigned_load_o),
 .branch_o            (branch_o),
 .branch_target_o     (branch_target_o),
 .discard_request_i   (discard_request_i)
);

endmodule // top
