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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <svdpi.h>

#include "Vtb_decm.h"
#include "testbench.h"
#include "Vtb_decm_ecap5_dproc_pkg.h"

class TB_Decm : public Testbench<Vtb_decm> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_decm>::reset();
  }
  
  void _nop() {
    core->input_valid_i = 0;
    core->instr_i = 0;
    core->pc_i = 0;
    core->output_ready_i = 0;
  }

  uint32_t sign_extend(uint32_t data, uint32_t nb_bits) {
    data &= (1 << nb_bits)-1;
    if((data >> (nb_bits-1)) & 0x1){
      data |= (((1 << (32 - (nb_bits-1))) - 1) << nb_bits);
    }
    return data;
  }

  uint32_t instr_r(uint32_t opcode, uint32_t rd, uint32_t func3, uint32_t rs1, uint32_t rs2, uint32_t func7) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (rd & 0x1F) << 7;
    instr |= (func3 & 0x7) << 12;
    instr |= (rs1 & 0x1F) << 15;
    instr |= (rs2 & 0x1F) << 20;
    instr |= (func7 & 0x7F) << 25;
    return instr;
  }

  uint32_t instr_i(uint32_t opcode, uint32_t rd, uint32_t func3, uint32_t rs1, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (rd & 0x1F) << 7;
    instr |= (func3 & 0x7) << 12;
    instr |= (rs1 & 0x1F) << 15;
    instr |= (imm & 0xFFF) << 20;
    return instr;
  }

  uint32_t instr_s(uint32_t opcode, uint32_t func3, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (imm & 0x1F) << 7;
    instr |= (func3 & 0x7) << 12;
    instr |= (rs1 & 0x1F) << 15;
    instr |= (rs2 & 0x1F) << 20;
    instr |= ((imm >> 5) & 0x7F) << 25;
    return instr;
  }

  uint32_t instr_b(uint32_t opcode, uint32_t func3, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= ((imm >> 11) & 1) << 7;
    instr |= ((imm >> 1) & 0xF) << 8;
    instr |= (func3 & 0x7) << 12;
    instr |= (rs1 & 0x1F) << 15;
    instr |= (rs2 & 0x1F) << 20;
    instr |= ((imm >> 5) & 0x3F) << 25;
    instr |= ((imm >> 12) & 1) << 31;
    return instr;
  }

  uint32_t instr_u(uint32_t opcode, uint32_t rd, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (rd & 0x1F) << 7;
    instr |= ((imm >> 12) & 0xFFFFF) << 12;
    return instr;
  }

  uint32_t instr_j(uint32_t opcode, uint32_t rd, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (rd & 0x1F) << 7;
    instr |= ((imm >> 12) & 0xFF) << 12;
    instr |= ((imm >> 11) & 1) << 20;
    instr |= ((imm >> 1) & 0x3FF) << 21;
    instr |= ((imm >> 20) & 1) << 31;
    return instr;
  }

  void _lui(uint32_t pc, uint32_t rd, uint32_t imm) {
    core->instr_i = instr_u(Vtb_decm_ecap5_dproc_pkg::OPCODE_LUI, rd, imm << 12);
    core->pc_i = pc;
  }

  void _auipc(uint32_t pc, uint32_t rd, uint32_t imm) {
    core->instr_i = instr_u(Vtb_decm_ecap5_dproc_pkg::OPCODE_AUIPC, rd, imm << 12);
    core->pc_i = pc;
  }

  void _jal(uint32_t pc, uint32_t rd, uint32_t imm) {
    core->instr_i = instr_j(Vtb_decm_ecap5_dproc_pkg::OPCODE_JAL, rd, imm);
    core->pc_i = pc;
  }

  void _jalr(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_JALR, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_JALR, rs1, imm);
    core->pc_i = pc;
  }

  void _beq(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BEQ, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _bne(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BNE, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _blt(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BLT, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _bge(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BGE, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _bltu(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BLTU, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _bgeu(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_b(Vtb_decm_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_decm_ecap5_dproc_pkg::FUNC3_BGEU, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _lb(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_LB, rs1, imm);
    core->pc_i = pc;
  }

  void _lbu(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_LBU, rs1, imm);
    core->pc_i = pc;
  }

  void _lh(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_LH, rs1, imm);
    core->pc_i = pc;
  }

  void _lhu(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_LHU, rs1, imm);
    core->pc_i = pc;
  }

  void _lw(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_LW, rs1, imm);
    core->pc_i = pc;
  }

  void _sb(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_s(Vtb_decm_ecap5_dproc_pkg::OPCODE_STORE, Vtb_decm_ecap5_dproc_pkg::FUNC3_SB, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _sh(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_s(Vtb_decm_ecap5_dproc_pkg::OPCODE_STORE, Vtb_decm_ecap5_dproc_pkg::FUNC3_SH, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _sw(uint32_t pc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
    core->instr_i = instr_s(Vtb_decm_ecap5_dproc_pkg::OPCODE_STORE, Vtb_decm_ecap5_dproc_pkg::FUNC3_SW, rs1, rs2, imm);
    core->pc_i = pc;
  }

  void _addi(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_ADD, rs1, imm);
    core->pc_i = pc;
  }

  void _slti(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLT, rs1, imm);
    core->pc_i = pc;
  }

  void _sltiu(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLTU, rs1, imm);
    core->pc_i = pc;
  }

  void _xori(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_XOR, rs1, imm);
    core->pc_i = pc;
  }

  void _ori(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_OR, rs1, imm);
    core->pc_i = pc;
  }

  void _andi(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_AND, rs1, imm);
    core->pc_i = pc;
  }

  void _slli(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLL, rs1, imm);
    core->pc_i = pc;
  }

  void _srli(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    // set func7
    uint32_t imm_w_func7 = (imm & 0x1F) | ((1 << 6) << 25);
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SRL, rs1, imm_w_func7);
    core->pc_i = pc;
  }

  void _srai(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t imm) {
    // set func7
    uint32_t imm_w_func7 = (imm & 0x1F) | ((1 << 5) << 5);
    core->instr_i = instr_i(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SRL, rs1, imm_w_func7);
    core->pc_i = pc;
  }

  void _add(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_ADD, rs1, rs2, Vtb_decm_ecap5_dproc_pkg::FUNC7_ADD);
    core->pc_i = pc;
  }

  void _sub(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_ADD, rs1, rs2, Vtb_decm_ecap5_dproc_pkg::FUNC7_SUB);
    core->pc_i = pc;
  }

  void _slt(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLT, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _sltu(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLTU, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _xor(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_XOR, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _or(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_OR, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _and(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_AND, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _sll(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SLL, rs1, rs2, 0);
    core->pc_i = pc;
  }

  void _srl(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SRL, rs1, rs2, Vtb_decm_ecap5_dproc_pkg::FUNC7_SRL);
    core->pc_i = pc;
  }

  void _sra(uint32_t pc, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    core->instr_i = instr_r(Vtb_decm_ecap5_dproc_pkg::OPCODE_OP, rd, Vtb_decm_ecap5_dproc_pkg::FUNC3_SRL, rs1, rs2, Vtb_decm_ecap5_dproc_pkg::FUNC7_SRA);
    core->pc_i = pc;
  }
};

enum CondId {
  COND_input_ready,
  COND_pc,
  COND_alu,
  COND_branch,
  COND_writeback,
  COND_loadstore,
  COND_output_valid,
  __CondIdEnd
};

void tb_decm_lui(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LUI 
  //    tick 1. Nothing (core outputs result of LUI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t imm = rand() % 32;
  tb->_lui(pc, rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  tb->check(COND_alu,       (core->alu_operand1_o  ==  pc)                                &&
                            (core->alu_operand2_o  ==  (imm << 12))                       &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lui.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lui.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lui.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lui.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_auipc(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 2;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for AUIPC
  //    tick 1. Nothing (core outputs result of AUIPC)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFFF - 10));
  tb->_auipc(pc, rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  pc)                                &&
                            (core->alu_operand2_o  ==  (imm << 12))                       &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.auipc.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.auipc.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.auipc.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.auipc.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_jal(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 3;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for JAL
  //    tick 1. Nothing (core outputs result of JAL)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFFF - 10)) & ~(0x1);
  tb->_jal(pc, rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_alu,       (core->alu_operand1_o  ==  pc)                                &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 21))          &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_UNCOND));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.jal.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.jal.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.jal.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.jal.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_jalr(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 4;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for JALR
  //    tick 1. Nothing (core outputs result of JALR)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_jalr(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)                            &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))          &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_UNCOND));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.jalr.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.jalr.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.jalr.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.jalr.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_beq(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 5;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BEQ
  //    tick 1. Nothing (core outputs result of BEQ)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_beq(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BEQ) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.beq.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.beq.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.beq.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.beq.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.beq.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_bne(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 6;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BNE
  //    tick 1. Nothing (core outputs result of BNE)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_bne(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BNE) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.bne.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.bne.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.bne.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.bne.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.bne.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_blt(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 7;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BLT
  //    tick 1. Nothing (core outputs result of BLT)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_blt(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BLT) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.blt.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.blt.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.blt.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.blt.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.blt.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_bge(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 8;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BGE
  //    tick 1. Nothing (core outputs result of BGE)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_bge(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BGE) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.bge.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.bge.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.bge.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.bge.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.bge.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_bltu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 9;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BLTU
  //    tick 1. Nothing (core outputs result of BLTU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_bltu(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BLTU) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.bltu.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.bltu.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.bltu.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.bltu.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.bltu.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_bgeu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 10;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BGEU
  //    tick 1. Nothing (core outputs result of BGEU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFF - 10)) & ~(0x1) ;
  tb->_bgeu(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_pc,        (core->pc_o            ==  pc));
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)          &&
                            (core->alu_operand2_o  ==  rdata2));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::BRANCH_BGEU) &&
                            (core->branch_offset_o ==  (tb->sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.bgeu.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decm.bgeu.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.bgeu.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.bgeu.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.bgeu.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_lb(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 11;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LB
  //    tick 1. Nothing (core outputs result of LB)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_lb(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x1) &&
                            (core->ls_unsigned_load_o ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lb.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lb.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lb.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lb.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_lbu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 12;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LBU
  //    tick 1. Nothing (core outputs result of LBU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_lbu(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x1) &&
                            (core->ls_unsigned_load_o ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lbu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lbu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lbu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lbu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_lh(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 13;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LH
  //    tick 1. Nothing (core outputs result of LH)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_lh(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x3) &&
                            (core->ls_unsigned_load_o ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lh.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lh.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lh.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lh.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_lhu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 14;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LHU
  //    tick 1. Nothing (core outputs result of LHU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_lhu(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x3) &&
                            (core->ls_unsigned_load_o ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lhu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lhu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lhu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lhu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_lw(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 15;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LW
  //    tick 1. Nothing (core outputs result of LW)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_lw(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0xF));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.lw.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.lw.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.lw.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.lw.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sb(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 16;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SB
  //    tick 1. Nothing (core outputs result of SB)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_sb(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0x1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sb.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sb.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sb.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sb.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sh(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 17;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SH
  //    tick 1. Nothing (core outputs result of SH)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_sh(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0x3));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sh.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sh.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sh.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sh.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sw(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 18;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SW
  //    tick 1. Nothing (core outputs result of SW)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_sw(pc, rs1, rs2, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  tb->sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0xF));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sw.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sw.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sw.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sw.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_addi(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 19;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ADDI
  //    tick 1. Nothing (core outputs result of ADDI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_addi(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.addi.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.addi.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.addi.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.addi.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_slti(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 20;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLTI
  //    tick 1. Nothing (core outputs result of SLTI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_slti(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_SLT));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.slti.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.slti.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.slti.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.slti.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sltiu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 21;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLTIU
  //    tick 1. Nothing (core outputs result of SLTIU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_sltiu(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_SLTU));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sltiu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sltiu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sltiu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sltiu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_xori(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 22;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for XORI
  //    tick 1. Nothing (core outputs result of XORI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_xori(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_XOR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.xori.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.xori.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.xori.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.xori.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_ori(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 23;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ORI
  //    tick 1. Nothing (core outputs result of ORI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_ori(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.ori.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.ori.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.ori.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.ori.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_andi(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 24;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ANDI
  //    tick 1. Nothing (core outputs result of ANDI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  tb->_andi(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  tb->sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.andi.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.andi.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.andi.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.andi.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_slli(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 25;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLLI
  //    tick 1. Nothing (core outputs result of SLLI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  tb->_slli(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o   ==  rdata1)  &&
                            (core->alu_operand2_o   ==  imm)     &&
                            (core->alu_op_o         ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o    ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o      ==  1) &&
                            (core->reg_addr_o       ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o      ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.slli.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.slli.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.slli.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.slli.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_srli(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 26;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRLI
  //    tick 1. Nothing (core outputs result of SRLI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  tb->_srli(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)  &&
                            (core->alu_operand2_o     ==  imm)     &&
                            (core->alu_op_o           ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  0));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.srli.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.srli.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.srli.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.srli.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_srai(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 27;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRAI
  //    tick 1. Nothing (core outputs result of SRAI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  tb->_srai(pc, rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)  &&
                            (core->alu_operand2_o     ==  imm)     &&
                            (core->alu_op_o           ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.srai.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.srai.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.srai.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.srai.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_add(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 28;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ADD
  //    tick 1. Nothing (core outputs result of ADD)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_add(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.add.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.add.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.add.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.add.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sub(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 29;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SUB
  //    tick 1. Nothing (core outputs result of SUB)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_sub(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  1));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sub.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sub.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sub.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sub.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_slt(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 30;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLT
  //    tick 1. Nothing (core outputs result of SLT)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_slt(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_SLT));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.slt.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.slt.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.slt.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.slt.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sltu(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 31;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLTU
  //    tick 1. Nothing (core outputs result of SLTU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_sltu(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_SLTU));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sltu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sltu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sltu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sltu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_xor(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 32;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for XOR
  //    tick 1. Nothing (core outputs result of XOR)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_xor(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_XOR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.xor.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.xor.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.xor.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.xor.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_or(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 33;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for OR
  //    tick 1. Nothing (core outputs result of OR)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_or(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.or.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.or.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.or.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.or.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_and(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 34;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for AND
  //    tick 1. Nothing (core outputs result of AND)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_and(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decm_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.and.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.and.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.and.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.and.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sll(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 35;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SLL
  //    tick 1. Nothing (core outputs result of SLL)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_sll(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o   ==  rdata1)  &&
                            (core->alu_operand2_o   ==  rdata2)  &&
                            (core->alu_op_o         ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o    ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o      ==  1) &&
                            (core->reg_addr_o       ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o      ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sll.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sll.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sll.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sll.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_srl(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 36;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRL
  //    tick 1. Nothing (core outputs result of SRL)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_srl(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)  &&
                            (core->alu_operand2_o     ==  rdata2)  &&
                            (core->alu_op_o           ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  0));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.srl.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.srl.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.srl.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.srl.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

void tb_decm_sra(TB_Decm * tb) {
  Vtb_decm * core = tb->core;
  core->testcase = 37;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRA
  //    tick 1. Nothing (core outputs result of SRA)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  tb->_sra(pc, rd, rs1, rs2);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;
  uint32_t rdata2 = rand() % 0x7FFFFFFF;
  core->rdata2_i = rdata2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)  &&
                            (core->alu_operand2_o     ==  rdata2)  &&
                            (core->alu_op_o           ==  Vtb_decm_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decm_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decm.sra.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decm.sra.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decm.sra.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decm.sra.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Decm * tb = new TB_Decm;
  tb->open_trace("waves/decm.vcd");
  tb->open_testdata("testdata/decm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_decm_lui(tb);
  tb_decm_auipc(tb);
  tb_decm_jal(tb);
  tb_decm_jalr(tb);
  tb_decm_beq(tb);
  tb_decm_bne(tb);
  tb_decm_blt(tb);
  tb_decm_bge(tb);
  tb_decm_bltu(tb);
  tb_decm_bgeu(tb);
  tb_decm_lb(tb);
  tb_decm_lbu(tb);
  tb_decm_lh(tb);
  tb_decm_lhu(tb);
  tb_decm_lw(tb);
  tb_decm_sb(tb);
  tb_decm_sh(tb);
  tb_decm_sw(tb);
  tb_decm_addi(tb);
  tb_decm_slti(tb);
  tb_decm_sltiu(tb);
  tb_decm_xori(tb);
  tb_decm_ori(tb);
  tb_decm_andi(tb);
  tb_decm_slli(tb);
  tb_decm_srli(tb);
  tb_decm_srai(tb);
  tb_decm_add(tb);
  tb_decm_sub(tb);
  tb_decm_slt(tb);
  tb_decm_sltu(tb);
  tb_decm_xor(tb);
  tb_decm_or(tb);
  tb_decm_and(tb);
  tb_decm_sll(tb);
  tb_decm_srl(tb);
  tb_decm_sra(tb);

  /************************************************************/

  printf("[DECM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
