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

module decode import ecap5_dproc_pkg::*;
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

  //=================================
  //    Hazard interface
  
  input    logic   stall_request_i

);

/*****************************************/
/*           Internal signals            */
/*****************************************/

logic[6:0] opcode;
logic[4:0] rd;
logic[2:0] func3;
logic[31:0] immediate;

logic[2:0] branch_cond;
logic[2:0] op_alu_op;

/*****************************************/
/*             Stage outputs             */
/*****************************************/

logic[31:0]  pc_q;

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
logic[31:0]  ls_write_data_d,     ls_write_data_q;
logic[3:0]   ls_sel_d,            ls_sel_q;
logic        ls_unsigned_load_d,  ls_unsigned_load_q;

logic        output_valid_d,      output_valid_q;

/*****************************************/

assign  opcode  =  instr_i[6:0];
assign  rd      =  instr_i[11:7];
assign  func3   =  instr_i[14:12];

assign raddr1_o = instr_i[19:15];
assign raddr2_o = instr_i[24:20];

always_comb begin : immediate_decoding
  immediate = '0;
  case(opcode)
    // I encoding
    OPCODE_JALR,
    OPCODE_LOAD:   immediate = { {21{instr_i[31]}}, instr_i[30:20] };
    OPCODE_OP_IMM: immediate = ((func3 == FUNC3_SLL) || (func3 == FUNC3_SRL))
                                      ? { 27'h0, instr_i[24:20] }
                                      : { {21{instr_i[31]}}, instr_i[30:20] };
    // S encoding
    OPCODE_STORE:  immediate = { {21{instr_i[31]}}, instr_i[30:25], instr_i[11:7] };
    // B encoding
    OPCODE_BRANCH: immediate = { {20{instr_i[31]}}, instr_i[7], instr_i[30:25], instr_i[11:8], 1'b0};
    // U encoding
    OPCODE_LUI, 
    OPCODE_AUIPC:  immediate = { instr_i[31:12], 12'h0 };
    // J encoding
    OPCODE_JAL:    immediate = { {12{instr_i[31]}}, instr_i[19:12], instr_i[20], instr_i[30:21], 1'b0};
    default: begin
    end
  endcase
end

always_comb begin : alu_interface
  case(opcode)
    OPCODE_LUI, OPCODE_AUIPC, OPCODE_JAL:                  
      alu_operand1_d = pc_i;
    OPCODE_JALR, OPCODE_BRANCH, OPCODE_OP, OPCODE_OP_IMM, OPCODE_LOAD, OPCODE_STORE:  
      alu_operand1_d = rdata1_i;
    default:                                               
      alu_operand1_d = '0;
  endcase

  case(opcode)
    OPCODE_LUI,
    OPCODE_AUIPC,
    OPCODE_JAL,
    OPCODE_JALR,
    OPCODE_OP_IMM,
    OPCODE_LOAD,
    OPCODE_STORE: alu_operand2_d = immediate;
    OPCODE_BRANCH,
    OPCODE_OP:     alu_operand2_d = rdata2_i;
    default:       alu_operand2_d = '0;
  endcase

  case(func3)
    FUNC3_ADD:            op_alu_op = ALU_ADD; 
    FUNC3_SLT:            op_alu_op = ALU_SLT; 
    FUNC3_SLTU:           op_alu_op = ALU_SLTU; 
    FUNC3_XOR:            op_alu_op = ALU_XOR; 
    FUNC3_OR:             op_alu_op = ALU_OR; 
    FUNC3_AND:            op_alu_op = ALU_AND; 
    FUNC3_SLL, FUNC3_SRL: op_alu_op = ALU_SHIFT; 
    default:              op_alu_op = ALU_ADD;
  endcase
  case(opcode)
    OPCODE_OP,
    OPCODE_OP_IMM: alu_op_d = op_alu_op;
    default:       alu_op_d = '0;
  endcase

  alu_shift_left_d = (func3 == FUNC3_SLL);
  alu_signed_shift_d = (instr_i[30] == 1'b1);
  alu_sub_d = (opcode == OPCODE_OP) && (instr_i[30] == 1'b1);
end

always_comb begin : branch_interface
  case(func3)
    FUNC3_BEQ:  branch_cond = BRANCH_BEQ;
    FUNC3_BNE:  branch_cond = BRANCH_BNE;
    FUNC3_BLT:  branch_cond = BRANCH_BLT;
    FUNC3_BGE:  branch_cond = BRANCH_BGE;
    FUNC3_BLTU: branch_cond = BRANCH_BLTU;
    FUNC3_BGEU: branch_cond = BRANCH_BGEU;
    default:    branch_cond = NO_BRANCH;
  endcase

  if(opcode == OPCODE_BRANCH) begin
    branch_cond_d = branch_cond;
  end else if((opcode == OPCODE_JAL) || (opcode == OPCODE_JALR)) begin
    branch_cond_d = BRANCH_UNCOND;
  end else begin
    branch_cond_d = NO_BRANCH;
  end

  branch_offset_d = immediate[19:0];
end

always_comb begin : writeback_interface
  reg_write_d = !((opcode == OPCODE_STORE) || (opcode == OPCODE_BRANCH));
  reg_addr_d = rd;
end

always_comb begin : loadstore_interface
  ls_enable_d = (opcode == OPCODE_LOAD) || (opcode == OPCODE_STORE);
  ls_write_d = (opcode == OPCODE_STORE);
  ls_write_data_d = rdata2_i;
  case(func3[1:0])
    2'b00:   ls_sel_d = 4'b0001;
    2'b01:   ls_sel_d = 4'b0011;
    2'b10:   ls_sel_d = 4'b1111;
    default: ls_sel_d = '0;
  endcase
  ls_unsigned_load_d = (func3 == FUNC3_LBU) || (func3 == FUNC3_LHU);
end

always_comb begin : output_handshake
  output_valid_d = output_valid_q;
  if(output_ready_i && ~stall_request_i) begin
    output_valid_d = 1;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    alu_operand1_q      <=  '0;
    alu_operand2_q      <=  '0;
    alu_op_q            <=  '0;
    alu_sub_q           <=   0;
    alu_shift_left_q    <=   0;
    alu_signed_shift_q  <=   0;

    branch_cond_q       <=  '0;
    branch_offset_q     <=  '0;

    reg_write_q         <=   0;
    reg_addr_q          <=  '0;

    ls_enable_q         <=   0;
    ls_write_q          <=   0;
    ls_write_data_q     <=  '0;
    ls_sel_q            <=  '0;
    ls_unsigned_load_q  <=   0;

    output_valid_q      <=   0;
  end else begin
    if(output_ready_i) begin
      pc_q                <=  pc_i;

      alu_operand1_q      <=  input_valid_i ? alu_operand1_d : '0;
      alu_operand2_q      <=  input_valid_i ? alu_operand2_d : '0;
      alu_op_q            <=  input_valid_i ? alu_op_d : ALU_ADD;
      alu_sub_q           <=  input_valid_i ? alu_sub_d : 0;
      alu_shift_left_q    <=  alu_shift_left_d;
      alu_signed_shift_q  <=  alu_signed_shift_d;

      branch_cond_q       <=  input_valid_i ? branch_cond_d : NO_BRANCH;
      branch_offset_q     <=  branch_offset_d;

      reg_write_q         <=  input_valid_i ? reg_write_d : 0;
      reg_addr_q          <=  reg_addr_d;

      ls_enable_q         <=  input_valid_i ? ls_enable_d : 0;
      ls_write_q          <=  ls_write_d;
      ls_write_data_q     <=  ls_write_data_d;
      ls_sel_q            <=  ls_sel_d;
      ls_unsigned_load_q  <=  ls_unsigned_load_d;
    end

    output_valid_q    <= output_valid_d;
  end
end

/*****************************************/
/*         Assign output signals         */
/*****************************************/

assign  input_ready_o       =  output_ready_i && ~stall_request_i;

assign  pc_o                =  pc_q;

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
assign  ls_write_data_o     =  ls_write_data_q;
assign  ls_sel_o            =  ls_sel_q;
assign  ls_unsigned_load_o    =  ls_unsigned_load_q;

assign  output_valid_o = output_valid_q;

endmodule // decode
