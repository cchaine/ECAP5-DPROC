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

module decm import ecap5_dproc_pkg::*;
(
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
  input   logic[31:0]   rdata1_o,
  output  logic[4:0]    raddr2_o,
  input   logic[31:0]   rdata2_o,

  //=================================
  //    Output logic
   
  input   logic         output_ready_i,
  output  logic         output_valid_o,

  //`````````````````````````````````
  //    Execute interface 
   
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
  output   logic[3:0]   ls_sel_o
);

/*****************************************/
/*           Internal signals            */
/*****************************************/
logic[6:0] opcode;
logic[31:0] immediate;

/*****************************************/
/*             Stage outputs             */
/*****************************************/

logic        input_ready_q;

logic[31:0]  alu_operand1_d,      alu_operand1_q;
logic[31:0]  alu_operand2_d,      alu_operand2_q;      
logic[2:0]   alu_op_d,            alu_op_q;
logic        alu_sub_d,           alu_sub_q;
logic        alu_shift_left_d,    alu_shift_left_q;
logic        alu_signed_shift_d,  alu_signed_shift_q;

logic[2:0]   branch_cond_d,       branch_cond_q;
logic[19:0]  branch_offset_d,     branch_offset_q;

logic        reg_write_d,         reg_write_q;
logic[4:0]   reg_addr_d,          reg_addr_q;

logic        ls_enable_d,         ls_enable_q;
logic        ls_write_d,          ls_write_q;
logic[3:0]   ls_sel_d,            ls_sel_q;

logic        output_valid_d,      output_valid_q;

/*****************************************/

assign opcode = instr_i[6:0];

always_comb begin : compute_immediate
  immediate = '0;
  case(opcode)
    OPCODE_LUI, 
    OPCODE_AUIPC: immediate = {instr_i[31:12], 12'h0};
    default: begin
    end
  endcase
end

always_comb begin : alu_interface
  alu_operand1_d = 0;
  alu_operand2_d = 0;
  alu_op_d = ALU_ADD;

  case(opcode)
    OPCODE_LUI: begin
      alu_operand1_d = immediate;
      alu_operand2_d = 0;
      alu_op_d = ALU_ADD;
    end
    default: begin
    end
  endcase
end

always_comb begin : branch_interface
  branch_cond_d = NO_BRANCH;
end

always_comb begin : register_interface
  raddr1_o = instr_i[19:15]; 
  raddr2_o = instr_i[24:20];
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    input_ready_q       <=   0;

    output_valid_q      <=   0;
  end else begin
    input_ready_q       <= output_ready_i;

    if(output_ready_i) begin
      alu_operand1_q      <=  alu_operand1_d;
      alu_operand2_q      <=  alu_operand2_d;
      alu_op_q            <=  alu_op_d;
      alu_sub_q           <=  alu_sub_d;
      alu_shift_left_q    <=  alu_shift_left_d;
      alu_signed_shift_q  <=  alu_signed_shift_d;

      branch_cond_q       <=  branch_cond_d;
      branch_offset_q     <=  branch_offset_d;

      reg_write_q         <=  reg_write_d;
      reg_addr_q          <=  reg_addr_d;

      ls_enable_q         <=  ls_enable_d;
      ls_write_q          <=  ls_write_d;
      ls_sel_q            <=  ls_sel_d;
    end

    output_valid_q    <= output_valid_d;
  end
end

assign  input_ready_o       =  input_ready_q;

assign  alu_operand1_o      =  alu_operand1_q;
assign  alu_operand2_o      =  alu_operand2_q;
assign  alu_op_o            =  alu_op_q;
assign  alu_sub_o           =  alu_sub_q;
assign  alu_shift_left_o    =  alu_shift_left_q;
assign  alu_signed_shift_o  =  alu_signed_shift_q;

assign  branch_cond_o       =  branch_cond_q;
assign  branch_offset_o     =  branch_offset_q;

assign  reg_write_o         =  reg_write_q;
assign  reg_addr_o          =  reg_addr_q;

assign  ls_enable_o         =  ls_enable_q;
assign  ls_write_o          =  ls_write_q;
assign  ls_sel_o            =  ls_sel_q;

endmodule // decm
