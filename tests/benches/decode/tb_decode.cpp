/*           __        _
 *  ________/ /  ___ _(_)__  ___
 * / __/ __/ _ \/ _ `/ / _ \/ -_)
 * \__/\__/_//_/\_,_/_/_//_/\__/
 * 
 * Copyright (C) Clément Chaine
 * This file is part of ECAP5-DPROC <https://github.com/ecap5/ECAP5-DPROC>
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

#include "Vtb_decode.h"
#include "testbench.h"
#include "riscv.h"
#include "Vtb_decode_ecap5_dproc_pkg.h"
#include "Vtb_decode_riscv_pkg.h"

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

enum TestcaseId {
  T_LUI             =  0,
  T_AUIPC           =  1,
  T_JAL             =  2,
  T_JALR            =  3,
  T_BEQ             =  4,
  T_BNE             =  5,
  T_BLT             =  6,
  T_BGE             =  7,
  T_BLTU            =  8,
  T_BGEU            =  9,
  T_LB              =  10,
  T_LBU             =  11,
  T_LH              =  12,
  T_LHU             =  13,
  T_LW              =  14,
  T_SB              =  15,
  T_SH              =  16,
  T_SW              =  17,
  T_ADDI            =  18,
  T_SLTI            =  19,
  T_SLTIU           =  20,
  T_XORI            =  21,
  T_ORI             =  22,
  T_ANDI            =  23,
  T_SLLI            =  24,
  T_SRLI            =  25,
  T_SRAI            =  26,
  T_ADD             =  27,
  T_SUB             =  28,
  T_SLT             =  29,
  T_SLTU            =  30,
  T_XOR             =  31,
  T_OR              =  32,
  T_AND             =  33,
  T_SLL             =  34,
  T_SRL             =  35,
  T_SRA             =  36,
  T_BUBBLE          =  37,
  T_PIPELINE_WAIT   =  38,
  T_HAZARD          =  39,
  T_RESET           =  40
};

class TB_Decode : public Testbench<Vtb_decode> {
public:
  void reset() {
    this->_nop();
    this->core->input_valid_i = 0;
    this->core->output_ready_i = 0;

    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_decode>::reset();
  }
  
  void _nop() {
    core->input_valid_i = 0;
    core->instr_i = 0;
    core->pc_i = 0;
    core->output_ready_i = 0;
  }
};

void tb_decode_reset(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_RESET;

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 
   
  tb->check(COND_output_valid, (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_decode.reset.01",
      tb->conditions[COND_output_valid],
      "Failed to implement the output handshake", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lui(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LUI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t imm = rand() % 32;

  core->instr_i = instr_lui(rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  tb->check(COND_alu,       (core->alu_operand1_o  ==  0)                                 &&
                            (core->alu_operand2_o  ==  (imm << 12))                       &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lui.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lui.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lui.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lui.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lui.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_auipc(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_AUIPC;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFFF - 10));
  core->instr_i = instr_auipc(rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  pc)                                &&
                            (core->alu_operand2_o  ==  (imm << 12))                       &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.auipc.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.auipc.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.auipc.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.auipc.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.auipc.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_jal(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_JAL;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFFFF - 10)) & ~(0x1);
  core->instr_i = instr_jal(rd, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_alu,       (core->alu_operand1_o  ==  pc)                                &&
                            (core->alu_operand2_o  ==  sign_extend(imm, 21))          &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_UNCOND));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.jal.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.jal.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.jal.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.jal.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.jal.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_jalr(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_JALR;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_jalr(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)                            &&
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))          &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_UNCOND));
  tb->check(COND_writeback, (core->reg_write_o     ==  1)  &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.jalr.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.jalr.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.jalr.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.jalr.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.jalr.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_beq(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BEQ;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_beq(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BEQ) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.beq.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.beq.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.beq.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.beq.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.beq.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.beq.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_bne(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BNE;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_bne(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BNE) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.bne.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.bne.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.bne.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.bne.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.bne.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.bne.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_blt(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BLT;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_blt(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BLT) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.blt.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.blt.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.blt.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.blt.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.blt.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.blt.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_bge(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BGE;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_bge(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BGE) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.bge.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.bge.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.bge.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.bge.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.bge.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.bge.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_bltu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BLTU;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_bltu(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BLTU) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.bltu.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.bltu.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.bltu.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.bltu.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.bltu.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.bltu.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_bgeu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BGEU;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0x1FFF - 10)) & ~(0x1) ;
  core->instr_i = instr_bgeu(rs1, rs2, imm);

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
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::BRANCH_BGEU) &&
                            (core->branch_offset_o ==  (sign_extend(imm, 13) & 0xFFFFF)));
  tb->check(COND_writeback, (core->reg_write_o     ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.bgeu.01",
      tb->conditions[COND_pc],
      "Failed to implement the pc output", tb->err_cycles[COND_pc]);

  CHECK("tb_decode.bgeu.02",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.bgeu.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.bgeu.04",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.bgeu.05",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.bgeu.06",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lb(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LB;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lb(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x1) &&
                            (core->ls_unsigned_load_o ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lb.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lb.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lb.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lb.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lb.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lbu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LBU;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lbu(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x1) &&
                            (core->ls_unsigned_load_o ==  1));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lbu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lbu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lbu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lbu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lbu.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lh(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LH;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lh(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x3) &&
                            (core->ls_unsigned_load_o ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lh.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lh.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lh.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lh.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lh.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lhu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LHU;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lhu(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0x3) &&
                            (core->ls_unsigned_load_o ==  1));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lhu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lhu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lhu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lhu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lhu.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_lw(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_LW;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lw(rd, rs1, imm);

  uint32_t rdata1 = rand() % 0x7FFFFFFF;
  core->rdata1_i = rdata1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  rdata1)          &&
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  0) &&
                            (core->ls_sel_o           ==  0xF));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.lw.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.lw.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.lw.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.lw.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.lw.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sb(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SB;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_sb(rs1, rs2, imm);

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
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0x1));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sb.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sb.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sb.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sb.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sb.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sh(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SH;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_sh(rs1, rs2, imm);

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
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0x3));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sh.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sh.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sh.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sh.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sh.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sw(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SW;

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
  core->pc_i = pc;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_sw(rs1, rs2, imm);

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
                            (core->alu_operand2_o     ==  sign_extend(imm, 12)));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  1) &&
                            (core->ls_write_o         ==  1) &&
                            (core->ls_write_data_o    ==  rdata2) &&
                            (core->ls_sel_o           ==  0xF));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sw.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sw.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sw.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sw.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sw.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_addi(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_ADDI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_addi(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.addi.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.addi.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.addi.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.addi.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.addi.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_slti(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLTI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_slti(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_SLT));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.slti.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.slti.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.slti.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.slti.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.slti.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sltiu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLTIU;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_sltiu(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_SLTU));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sltiu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sltiu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sltiu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sltiu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sltiu.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_xori(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_XORI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_xori(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_XOR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.xori.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.xori.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.xori.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.xori.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.xori.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_ori(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_ORI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_ori(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.ori.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.ori.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.ori.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.ori.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.ori.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_andi(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_ANDI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_andi(rd, rs1, imm);

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
                            (core->alu_operand2_o  ==  sign_extend(imm, 12))  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.andi.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.andi.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.andi.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.andi.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.andi.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_slli(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLLI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  core->instr_i = instr_slli(rd, rs1, imm);

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
                            (core->alu_op_o         ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o    ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o      ==  1) &&
                            (core->reg_addr_o       ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o      ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.slli.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.slli.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.slli.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.slli.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.slli.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_srli(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SRLI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  core->instr_i = instr_srli(rd, rs1, imm);

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
                            (core->alu_op_o           ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  0));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.srli.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.srli.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.srli.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.srli.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.srli.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_srai(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SRAI;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = 1 + rand() % (0x1F - 1);
  core->instr_i = instr_srai(rd, rs1, imm);

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
                            (core->alu_op_o           ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.srai.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.srai.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.srai.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.srai.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.srai.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_add(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_ADD;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_add(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  0));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.add.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.add.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.add.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.add.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.add.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sub(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SUB;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_sub(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o       ==  1));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sub.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sub.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sub.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sub.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sub.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_slt(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLT;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_slt(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_SLT));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.slt.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.slt.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.slt.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.slt.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.slt.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sltu(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLTU;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_sltu(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_SLTU));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sltu.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sltu.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sltu.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sltu.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sltu.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_xor(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_XOR;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_xor(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_XOR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.xor.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.xor.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.xor.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.xor.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.xor.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_or(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_OR;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_or(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.or.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.or.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.or.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.or.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.or.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_and(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_AND;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_and(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.and.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.and.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.and.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.and.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.and.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sll(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SLL;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_sll(rd, rs1, rs2);

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
                            (core->alu_op_o         ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o    ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o      ==  1) &&
                            (core->reg_addr_o       ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o      ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sll.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sll.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sll.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sll.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sll.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_srl(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SRL;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_srl(rd, rs1, rs2);

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
                            (core->alu_op_o           ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  0));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.srl.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.srl.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.srl.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.srl.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.srl.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_sra(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_SRA;

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
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_sra(rd, rs1, rs2);

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
                            (core->alu_op_o           ==  Vtb_decode_ecap5_dproc_pkg::ALU_SHIFT) &&
                            (core->alu_shift_left_o   ==  0) &&
                            (core->alu_signed_shift_o ==  1));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  1) &&
                            (core->reg_addr_o         ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.sra.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.sra.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.sra.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.sra.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.sra.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_bubble(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_BUBBLE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LB without input valid
  //    tick 1. Nothing (core outputs bubble)
  
  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = (10 + rand() % (0xFFFF - 10));
  core->instr_i = instr_lb(rd, rs1, imm);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o     ==  0)  &&
                            (core->alu_operand2_o     ==  0)  &&
                            (core->alu_op_o           ==  Vtb_decode_ecap5_dproc_pkg::ALU_ADD) &&
                            (core->alu_sub_o          ==  0));
  tb->check(COND_branch,    (core->branch_cond_o      ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o        ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.bubble.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol bubble bypass", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.bubble.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol bubble bypass", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.bubble.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol bubble bypass", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.bubble.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol bubble bypass", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.bubble.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_pipeline_wait(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_PIPELINE_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for AND
  //    tick 1. Stall the pipeline (core holds outputs result of AND)
  //    tick 2. Nothing (core holds outputs result of AND)
  //    tick 3. Unstall pipeline (core holds outputs result of AND)
  //    tick 4. Set inputs for OR (core holds result of AND)
  //    tick 5. Nothing (core outputs result of OR)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_and(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 0;
  core->instr_i = instr_or(rd, rs1, rs2);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->instr_i = instr_and(rd, rs1, rs2);

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.pipeline_wait.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.pipeline_wait.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.pipeline_wait.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.pipeline_wait.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.pipeline_wait.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

void tb_decode_hazard(TB_Decode * tb) {
  Vtb_decode * core = tb->core;
  core->testcase = T_HAZARD;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for AND
  //    tick 1. Set hazard (core holds outputs result of AND)
  //    tick 2. Nothing (core holds outputs result of AND)
  //    tick 3. Unstall pipeline (core holds outputs result of AND)
  //    tick 4. Set inputs for OR (core holds result of AND)
  //    tick 5. Nothing (core outputs result of OR)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand();
  core->pc_i = pc;
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs2 = rand() % 32;
  core->instr_i = instr_and(rd, rs1, rs2);

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
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->stall_request_i = 1;
  core->instr_i = instr_or(rd, rs1, rs2);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  0) &&
                            (core->reg_addr_o      ==  0));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->stall_request_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_OR));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->instr_i = instr_and(rd, rs1, rs2);

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_alu,       (core->alu_operand1_o  ==  rdata1)  &&
                            (core->alu_operand2_o  ==  rdata2)  &&
                            (core->alu_op_o        ==  Vtb_decode_ecap5_dproc_pkg::ALU_AND));
  tb->check(COND_branch,    (core->branch_cond_o   ==  Vtb_decode_ecap5_dproc_pkg::NO_BRANCH));
  tb->check(COND_writeback, (core->reg_write_o     ==  1) &&
                            (core->reg_addr_o      ==  rd));
  tb->check(COND_loadstore, (core->ls_enable_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_decode.hazard.01",
      tb->conditions[COND_alu],
      "Failed to implement the alu protocol", tb->err_cycles[COND_alu]);

  CHECK("tb_decode.hazard.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_decode.hazard.03",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback protocol", tb->err_cycles[COND_writeback]);

  CHECK("tb_decode.hazard.04",
      tb->conditions[COND_loadstore],
      "Failed to implement the load-store protocol", tb->err_cycles[COND_loadstore]);

  CHECK("tb_decode.hazard.05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output valid signal", tb->err_cycles[COND_output_valid]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Decode * tb = new TB_Decode;
  tb->open_trace("waves/decode.vcd");
  tb->open_testdata("testdata/decode.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_decode_reset(tb);

  tb_decode_lui(tb);
  tb_decode_auipc(tb);
  tb_decode_jal(tb);
  tb_decode_jalr(tb);
  tb_decode_beq(tb);
  tb_decode_bne(tb);
  tb_decode_blt(tb);
  tb_decode_bge(tb);
  tb_decode_bltu(tb);
  tb_decode_bgeu(tb);
  tb_decode_lb(tb);
  tb_decode_lbu(tb);
  tb_decode_lh(tb);
  tb_decode_lhu(tb);
  tb_decode_lw(tb);
  tb_decode_sb(tb);
  tb_decode_sh(tb);
  tb_decode_sw(tb);
  tb_decode_addi(tb);
  tb_decode_slti(tb);
  tb_decode_sltiu(tb);
  tb_decode_xori(tb);
  tb_decode_ori(tb);
  tb_decode_andi(tb);
  tb_decode_slli(tb);
  tb_decode_srli(tb);
  tb_decode_srai(tb);
  tb_decode_add(tb);
  tb_decode_sub(tb);
  tb_decode_slt(tb);
  tb_decode_sltu(tb);
  tb_decode_xor(tb);
  tb_decode_or(tb);
  tb_decode_and(tb);
  tb_decode_sll(tb);
  tb_decode_srl(tb);
  tb_decode_sra(tb);

  tb_decode_bubble(tb);

  tb_decode_pipeline_wait(tb);

  tb_decode_hazard(tb);

  /************************************************************/

  printf("[DECODE]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
