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

module exm import ecap5_dproc_pkg::*;
(
  input   logic        clk_i,
  input   logic        rst_i,
  // Input handshake
  output  logic        input_ready_o,
  input   logic        input_valid_i,
  // PC
  input   logic[31:0]  pc_i,
  // ALU inputs 
  input   logic[31:0]  alu_operand1_i,
  input   logic[31:0]  alu_operand2_i, 
  input   logic[2:0]   alu_op_i,
  input   logic        alu_sub_i,
  input   logic        alu_shift_left_i,
  input   logic        alu_signed_shift_i,
  // Branch inputs 
  input   logic[2:0]   branch_cond_i,
  input   logic[19:0]  branch_offset_i,
  // Output write inputs
  input   logic        result_write_i,
  input   logic[4:0]   result_addr_i,
  // Output handshake
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  // Output write outputs
  output  logic        result_write_o,
  output  logic[4:0]   result_addr_o,
  // ALU output
  output  logic[31:0]  result_o,
  // Branch outputs
  output  logic        branch_o,
  output  logic[31:0]  branch_target_o
);

/*****************************************/
/*         ALU internal signals          */
/*****************************************/

logic        input_ready_q;

logic signed[31:0] alu_signed_operand1,
                   alu_signed_operand2;

logic[31:0] alu_sum_operand2;

logic[31:0] alu_shift0,
            alu_shift1,
            alu_shift2,
            alu_shift3,
            alu_shift4;
logic[31:0] alu_shift_operand1;
logic[31:0] alu_shift_fill;

logic[31:0] alu_sum_output, 
            alu_xor_output,
            alu_or_output,
            alu_and_output,
            alu_slt_output,
            alu_sltu_output,
            alu_shift_output;
logic[31:0] alu_output;
logic alu_sum_z;
logic[31:0] pc_next;

/*****************************************/
/*             Stage outputs             */
/*****************************************/

logic        result_write_q;
logic[4:0]   result_addr_q;
logic[31:0]  result_d, result_q;
logic        branch_d, branch_q;
logic[31:0]  branch_target_d, branch_target_q;
logic        output_valid_d, output_valid_q;

/*****************************************/

assign pc_next = pc_i + 32'h4;

always_comb begin : alu
  alu_signed_operand1 = $signed(alu_operand1_i);
  alu_signed_operand2 = $signed(alu_operand2_i);

  // The second alu operand is inverted in the following cases :
  //   . The requested alu operation is a substraction (alu_sub_q)
  //   . The requested branch operation is BEQ as the substraction is used to determine the equality
  //   . The requested branch operation is BNE as the substraction is used to determine the non-equality
  alu_sum_operand2 = alu_sub_i || (branch_cond_i == BRANCH_BEQ || branch_cond_i == BRANCH_BNE)
                          ? (-alu_signed_operand2)
                          :   alu_signed_operand2;
  alu_sum_output   =  alu_signed_operand1 + alu_sum_operand2;
  // A flag indicating if the output of the sum is zero is computed to be used with branch operations
  alu_sum_z = (alu_sum_output == 32'h0);

  alu_xor_output   =  alu_operand1_i  ^  alu_operand2_i;
  alu_or_output    =  alu_operand1_i  |  alu_operand2_i;
  alu_and_output   =  alu_operand1_i  &  alu_operand2_i;
  alu_slt_output   =  {31'h0, alu_signed_operand1 < alu_signed_operand2};
  alu_sltu_output  =  {31'h0,      alu_operand1_i <      alu_operand2_i};

  // The bitorder of the first shift operand is inverted in case of a left shift
  alu_shift_operand1 = alu_shift_left_i
                            ? {<<{alu_operand1_i}}
                            :     alu_operand1_i;
  // The bits are filled with ones instead of zero when the requested shift is a right signed shift (SRA) with
  // a negative operand.
  alu_shift_fill = (~alu_shift_left_i && alu_signed_shift_i && alu_operand1_i[31])
                        ? 32'hFFFFFFFF
                        : 32'h00000000;
  // Barrel shifter implementation
  alu_shift0  =  alu_operand2_i[0]  ?  {    alu_shift_fill[0],  alu_shift_operand1[31:1]}  :  alu_shift_operand1;
  alu_shift1  =  alu_operand2_i[1]  ?  {  alu_shift_fill[1:0],          alu_shift0[31:2]}  :          alu_shift0;
  alu_shift2  =  alu_operand2_i[2]  ?  {  alu_shift_fill[3:0],          alu_shift1[31:4]}  :          alu_shift1;
  alu_shift3  =  alu_operand2_i[3]  ?  {  alu_shift_fill[7:0],          alu_shift2[31:8]}  :          alu_shift2;
  alu_shift4  =  alu_operand2_i[4]  ?  { alu_shift_fill[15:0],         alu_shift3[31:16]}  :          alu_shift3;
  // The bitorder of the shift output is re-inverted in case of a left shift
  alu_shift_output = alu_shift_left_i
                          ? {<<{alu_shift4}}
                          :     alu_shift4;

  case(alu_op_i)
    ALU_ADD:    alu_output  =  alu_sum_output;
    ALU_XOR:    alu_output  =  alu_xor_output;
    ALU_OR:     alu_output  =  alu_or_output;
    ALU_AND:    alu_output  =  alu_and_output;
    ALU_SLT:    alu_output  =  alu_slt_output;
    ALU_SLTU:   alu_output  =  alu_sltu_output;
    ALU_SHIFT:  alu_output  =  alu_shift_output;
    default:    alu_output  =  '0;
  endcase
end

always_comb begin : result_mux
  result_d = (branch_cond_i == BRANCH_UNCOND)
                ? pc_next
                : alu_output;
end

always_comb begin : branch_interface
  case(branch_cond_i)
    NO_BRANCH:     branch_d  =   0;
    BRANCH_BEQ:    branch_d  =   alu_sum_z;
    BRANCH_BNE:    branch_d  =  ~alu_sum_z;
    BRANCH_BLT:    branch_d  =   alu_slt_output[0];
    BRANCH_BLTU:   branch_d  =   alu_sltu_output[0];
    BRANCH_BGE:    branch_d  =  ~alu_slt_output[0];
    BRANCH_BGEU:   branch_d  =  ~alu_sltu_output[0];
    BRANCH_UNCOND: branch_d  =   1;
    default:       branch_d  =   0;
  endcase

  branch_target_d = (branch_cond_i == BRANCH_UNCOND)
                        ? alu_sum_output
                        : (pc_i + {12'h0, branch_offset_i}); 
end

always_comb begin : output_handshake
  output_valid_d = output_valid_q;
  if(output_ready_i) begin
    output_valid_d = 1;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    input_ready_q       <=   0;

    result_write_q      <=   0;
    result_addr_q       <=  '0;
    branch_target_q     <=  '0;

    result_q            <=  '0;
    branch_q            <=   0;

    output_valid_q      <=   0;
  end else begin
    input_ready_q       <= output_ready_i;

    if(output_ready_i) begin
      result_write_q      <=  input_valid_i
                                  ? result_write_i
                                  : 0;
      result_addr_q       <=  result_addr_i;
      branch_target_q     <=  branch_target_d;

      result_q          <=  result_d;
      branch_q          <=  input_valid_i
                                  ? branch_d
                                  : 0;
    end

    output_valid_q    <= output_valid_d;
  end
end

/*****************************************/
/*         Assign output signals         */
/*****************************************/

assign input_ready_o = input_ready_q;

assign result_write_o = result_write_q;
assign result_addr_o = result_addr_q;
assign result_o = result_q;

assign branch_o = branch_q;
assign branch_target_o = branch_target_q;

assign output_valid_o = output_valid_q;

endmodule // exm
