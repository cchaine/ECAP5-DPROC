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
  output  logic[19:0]  branch_offset_o
);

// Registered inputs
logic[31:0]  alu_operand1_q,
             alu_operand2_q;  
logic[2:0]   alu_op_q;
logic        alu_sub_q;
logic        alu_shift_left_q;
logic        alu_signed_shift_q;
logic        result_write_q;
logic[4:0]   result_addr_q;
logic[2:0]   branch_cond_q;
logic[19:0]  branch_offset_q;
logic        input_ready_q;

// ALU internal signals
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
logic alu_sum_z;

// Stage outputs
logic        result_write_qq;     
logic[4:0]   result_addr_qq;  
logic[31:0]  result_d, result_q;
logic        branch_d, branch_q;
logic[19:0]  branch_offset_qq;    
logic        output_valid_d, output_valid_q;

always_comb begin : alu
  alu_signed_operand1 = $signed(alu_operand1_q);
  alu_signed_operand2 = $signed(alu_operand2_q);

  alu_sum_operand2 = alu_sub_q || (branch_cond_q == BRANCH_BEQ || branch_cond_q == BRANCH_BNE)
                          ? (-alu_signed_operand2)
                          :   alu_signed_operand2;
  alu_sum_output   =  alu_signed_operand1 + alu_sum_operand2;
  alu_sum_z = (alu_sum_output == 32'h0);

  alu_xor_output   =  alu_operand1_q  ^  alu_operand2_q;
  alu_or_output    =  alu_operand1_q  |  alu_operand2_q;
  alu_and_output   =  alu_operand1_q  &  alu_operand2_q;
  alu_slt_output   =  {31'h0, alu_signed_operand1 < alu_signed_operand2};
  alu_sltu_output  =  {31'h0,      alu_operand1_q <      alu_operand2_q};

  alu_shift_operand1 = alu_shift_left_q
                            ? {<<{alu_operand1_q}}
                            :     alu_operand1_q;
  alu_shift_fill = (alu_signed_shift_q && ~alu_shift_left_q && alu_operand1_q[31])
                        ? 32'hFFFFFFFF
                        : 32'h00000000;
  alu_shift0  =  alu_operand2_q[0]  ?  {    alu_shift_fill[0],  alu_shift_operand1[31:1]}  :  alu_shift_operand1;
  alu_shift1  =  alu_operand2_q[1]  ?  {  alu_shift_fill[1:0],          alu_shift0[31:2]}  :          alu_shift0;
  alu_shift2  =  alu_operand2_q[2]  ?  {  alu_shift_fill[3:0],          alu_shift1[31:4]}  :          alu_shift1;
  alu_shift3  =  alu_operand2_q[3]  ?  {  alu_shift_fill[7:0],          alu_shift2[31:8]}  :          alu_shift2;
  alu_shift4  =  alu_operand2_q[4]  ?  { alu_shift_fill[15:0],         alu_shift3[31:16]}  :          alu_shift3;
  alu_shift_output = alu_shift_left_q
                          ? {<<{alu_shift4}}
                          :     alu_shift4;
end

always_comb begin : result
  case(alu_op_q)
    ALU_ADD:    result_d  =  alu_sum_output;
    ALU_XOR:    result_d  =  alu_xor_output;
    ALU_OR:     result_d  =  alu_or_output;
    ALU_AND:    result_d  =  alu_and_output;
    ALU_SLT:    result_d  =  alu_slt_output;
    ALU_SLTU:   result_d  =  alu_sltu_output;
    ALU_SHIFT:  result_d  =  alu_shift_output;
    default:    result_d  =  '0;
  endcase
end

always_comb begin : branch
  case(branch_cond_q)
    NO_BRANCH:    branch_d  =   '0;
    BRANCH_BEQ:   branch_d  =   alu_sum_z;
    BRANCH_BNE:   branch_d  =  ~alu_sum_z;
    BRANCH_BLT:   branch_d  =   alu_slt_output[0];
    BRANCH_BLTU:  branch_d  =   alu_sltu_output[0];
    BRANCH_BGE:   branch_d  =  ~alu_slt_output[0];
    BRANCH_BGEU:  branch_d  =  ~alu_sltu_output[0];
    default:      branch_d  =   '0;
  endcase
end

always_comb begin : output_handshake
  output_valid_d = output_valid_q;
  if(input_ready_q) begin
    output_valid_d = 1;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i || ~input_valid_i) begin
    alu_operand1_q      <=  '0;
    alu_operand2_q      <=  '0;
    alu_op_q            <=   ALU_ADD;
    alu_sub_q           <=   0;
    alu_shift_left_q   <=    0;
    alu_signed_shift_q  <=   0;
    result_write_q      <=   0;
    result_addr_q       <=  '0;
    branch_cond_q       <=   NO_BRANCH;
    branch_offset_q     <=  '0;

    input_ready_q       <=   0;

    result_write_qq     <=   0;
    result_addr_qq      <=  '0;
    result_q            <=  '0;
    branch_q            <=   0;
    branch_offset_qq    <=  '0;

    output_valid_q      <=   0;
  end else begin
    // Input registering
    if(output_ready_i) begin
      alu_operand1_q      <=  alu_operand1_i;
      alu_operand2_q      <=  alu_operand2_i;
      alu_op_q            <=  alu_op_i;
      alu_sub_q           <=  alu_sub_i;
      alu_shift_left_q    <=  alu_shift_left_i;
      alu_signed_shift_q  <=  alu_signed_shift_i;
      result_write_q      <=  result_write_i;
      result_addr_q       <=  result_addr_i;
      branch_cond_q       <=  branch_cond_i;
      branch_offset_q     <=  branch_offset_i;
    end

    input_ready_q       <= output_ready_i;

    result_addr_qq    <=  result_addr_q;
    result_write_qq   <=  result_write_q;
    result_q          <=  result_d;
    branch_q          <=  branch_d;
    branch_offset_qq  <=  branch_offset_q;

    output_valid_q    <= output_valid_d;
  end
end
assign input_ready_o = input_ready_q;
assign result_write_o = result_write_qq;
assign result_addr_o = result_addr_qq;
assign result_o = result_q;
assign branch_o = branch_q;
assign branch_offset_o = branch_offset_qq;
assign output_valid_o = output_valid_q;

endmodule // exm
