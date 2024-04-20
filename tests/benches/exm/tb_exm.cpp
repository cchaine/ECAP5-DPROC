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

#include "Vtb_exm.h"
#include "testbench.h"
#include "Vtb_exm_ecap5_dproc_pkg.h"

enum CondId {
  COND_input_ready,
  COND_result,
  COND_branch,
  COND_output_valid,
  __CondIdEnd
};

enum TestcaseId {
  T_ALU_ADD                     =  1,
  T_ALU_SUB                     =  2,
  T_ALU_XOR                     =  3,
  T_ALU_OR                      =  4,
  T_ALU_AND                     =  5,
  T_ALU_SLT                     =  6,
  T_ALU_SLTU                    =  7,
  T_ALU_SLL                     =  8,
  T_ALU_SRL                     =  9,
  T_ALU_SRA                     =  10,
  T_BRANCH_BEQ                  =  11,
  T_BRANCH_BNE                  =  12,
  T_BRANCH_BLT                  =  13,
  T_BRANCH_BLTU                 =  14,
  T_BRANCH_BGE                  =  15,
  T_BRANCH_BGEU                 =  16,
  T_BACK_TO_BACK                =  17,
  T_BUBBLE                      =  18,
  T_PIPELINE_STALL_AFTER_RESET  =  19,
  T_PIPELINE_STALL              =  20,
  T_RESET                       =  21,
  T_BRANCH_JALR                 =  22
};

class TB_Exm : public Testbench<Vtb_exm> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_exm>::reset();
  }
  
  void _nop() {
    this->core->alu_operand1_i = 0;
    this->core->alu_operand2_i = 0;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
    this->core->alu_sub_i = 0;
    this->core->alu_shift_left_i = 0;
    this->core->alu_signed_shift_i = 0;
    this->core->reg_write_i = 0;
    this->core->reg_addr_i = 0;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::NO_BRANCH;
    this->core->branch_offset_i = 0;
  }

  void _add(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
    this->core->alu_sub_i = 0;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _sub(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
    this->core->alu_sub_i = 1;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _xor(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_XOR;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _or(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_OR;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _and(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_AND;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _slt(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_SLT;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _sltu(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_SLTU;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _sll(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_SHIFT;
    this->core->alu_shift_left_i = 1;
    this->core->alu_signed_shift_i = 0;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _srl(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_SHIFT;
    this->core->alu_shift_left_i = 0;
    this->core->alu_signed_shift_i = 0;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _sra(uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_SHIFT;
    this->core->alu_shift_left_i = 0;
    this->core->alu_signed_shift_i = 1;
    this->core->branch_cond_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _beq(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BEQ;
    this->core->branch_offset_i = branch_offset;
  }

  void _bne(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BNE;
    this->core->branch_offset_i = branch_offset;
  }

  void _blt(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BLT;
    this->core->branch_offset_i = branch_offset;
  }

  void _bltu(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BLTU;
    this->core->branch_offset_i = branch_offset;
  }

  void _bge(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BGE;
    this->core->branch_offset_i = branch_offset;
  }

  void _bgeu(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t branch_offset) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BGEU;
    this->core->branch_offset_i = branch_offset;
  }

  void _jalr(uint32_t pc, uint32_t operand1, uint32_t operand2, uint32_t reg_addr) {
    this->_nop();
    this->core->pc_i = pc;
    this->core->alu_operand1_i = operand1;
    this->core->alu_operand2_i = operand2;
    this->core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_UNCOND;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }
};

void tb_exm_alu_add(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_ADD;

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

  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_add(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.ADD_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.ADD_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.ADD_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_sub(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SUB;

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
  
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint32_t reg_addr = rand() % 32;
  tb->_sub(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  uint32_t result = ((int32_t)operand1 - (int32_t)operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_exm.alu.SUB_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SUB_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SUB_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_xor(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_XOR;

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
  
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_xor(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = (operand1 ^ operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.XOR_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.XOR_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.XOR_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_or(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_OR;

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
  
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_or(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = (operand1 | operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.OR_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.OR_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.OR_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_and(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_AND;

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
  
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_and(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = (operand1 & operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.AND_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.AND_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.AND_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_slt(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SLT;

  // The following actions are performed in this test :
  //    tick 0.    Set inputs for SLT starting with false
  //    tick 1-10. Set inputs for SLT crossing the false-true output

  //=================================
  //      Tick (0)

  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;
  
  // Test values from operand2 - 8 to operand2 + 2
  uint8_t reg_addr = rand() % 32;
  uint32_t operand2 = 2 + rand() % 5;
  uint32_t result;
  for(int i = 0; i < 10; i++) {
    tb->_slt((int32_t)operand2 - 8 + i, operand2, reg_addr);

    //=================================
    //      Tick (1 to 10)
    
    tb->tick();

    //`````````````````````````````````
    //      Checks 
    
    result = (((int32_t)operand2 - 8 + i) < (int32_t)operand2);
    tb->check(COND_result,       (core->result_o        ==  result) && 
                                 (core->reg_write_o  ==  1)      &&
                                 (core->reg_addr_o   ==  reg_addr));
    tb->check(COND_branch,       (core->branch_o        ==  0));
    tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  }

  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_exm.alu.SLT_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SLT_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SLT_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_sltu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SLTU;

  // The following actions are performed in this test :
  //    tick 0.    Set inputs for SLTU starting with false
  //    tick 1-10. Set inputs for SLTU crossing the false-true output

  //=================================
  //      Tick (0)

  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;
  
  // Test values from operand2 - 8 to operand2 + 2
  uint8_t reg_addr = rand() % 32;
  uint32_t operand2 = 2 + rand() % 5;
  uint32_t result;
  for(int i = 0; i < 10; i++) {
    tb->_sltu((int32_t)operand2 - 8 + i, operand2, reg_addr);

    //=================================
    //      Tick (1 to 10)
    
    tb->tick();

    //`````````````````````````````````
    //      Checks 
    
    result = ((uint32_t)((int32_t)operand2 - 8 + i) < (uint32_t)((int32_t)operand2));
    tb->check(COND_result,       (core->result_o        ==  result) && 
                                 (core->reg_write_o  ==  1)      &&
                                 (core->reg_addr_o   ==  reg_addr));
    tb->check(COND_branch,       (core->branch_o        ==  0));
    tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  }

  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_exm.alu.SLTU_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SLTU_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SLTU_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_sll(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SLL;

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
  
  uint32_t operand1 = rand();
  uint32_t operand2 = 3 + rand() % 29;
  uint32_t reg_addr = rand() % 32;
  tb->_sll(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = (operand1 << (operand2 & 0x1F));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.SLL_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SLL_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SLL_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_srl(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SRL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRL with negative number
  //    tick 1. Set inputs for SRL with positive number (core outputs result of SRL)
  //    tick 2. Nothing (core outputs result of SRL)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t operand1 = rand() | 0x80000000; // enable the sign bit
  uint32_t operand2 = 3 + rand() % 29;
  uint32_t reg_addr = rand() % 32;
  tb->_srl(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = (operand1 >> (operand2 & 0x1F));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  operand1 = rand() & ~(0x80000000); // disable the sign bit
  operand2 = 3 + rand() % 29;
  reg_addr = rand() % 32;
  tb->_srl(operand1, operand2, reg_addr);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = (operand1 >> (operand2 & 0x1F));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.SRL_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SRL_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SRL_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_alu_sra(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_ALU_SRA;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for SRA with negative number
  //    tick 1. Set inputs for SRA with positive number (core outputs result of SRA)
  //    tick 2. Nothing (core outputs result of SRA)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t operand1 = rand() | 0x80000000; // enable the sign bit
  uint32_t operand2 = 3 + rand() % 29;
  uint32_t reg_addr = rand() % 32;
  tb->_sra(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result  = (operand1 >> (operand2 & 0x1F));
           result |= ((1 << (operand2 & 0x1F)) - 1) << (32 - (operand2 & 0x1F));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  operand1 = rand() & ~(0x80000000); // disable the sign bit
  operand2 = 3 + rand() % 29;
  reg_addr = rand() % 32;
  tb->_sra(operand1, operand2, reg_addr);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = (operand1 >> (operand2 & 0x1F));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.alu.SRA_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.alu.SRA_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.alu.SRA_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_beq(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BEQ;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BEQ with different values
  //    tick 1. Set inputs for BEQ with equal values (core outputs result of BEQ)
  //    tick 2. Nothing (core outputs result of BEQ)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_beq(pc, operand1, operand2, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  tb->_beq(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.branch.BEQ_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BEQ_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BEQ_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_bne(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BNE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BNE with equal values
  //    tick 1. Set inputs for BNE with different values (core outputs result of BNE)
  //    tick 2. Nothing (core outputs result of BNE)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_bne(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  tb->_bne(pc, operand1, operand2, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.branch.BNE_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BNE_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BNE_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_blt(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BLT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BLT with greater value
  //    tick 1. Set inputs for BLT with equal value (core outputs result of BLT)
  //    tick 2. Set inputs for BLT with lower value (core outputs result of BLT)
  //    tick 3. Nothing (core outputs result of BLT)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = 2 + rand() % 6;
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_blt(pc, (int32_t)operand1 + 10, operand1, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  tb->_blt(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs

  tb->_blt(pc, (int32_t)operand1 - 10, operand1, branch_offset);

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.branch.BLT_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BLT_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BLT_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_bltu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BLTU;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BLTU with greater value
  //    tick 1. Set inputs for BLTU with equal values (core outputs result of BLTU)
  //    tick 2. Set inputs for BLTU with lower negative value (core outputs result of BLTU)
  //    tick 3. Set inputs for BLTU with lower postive value (core outputs result of BLTU)
  //    tick 4. Nothing (core outputs result of BLTU)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = 2 % rand() % 6;
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_bltu(pc, (int32_t)operand1 + 10, operand1, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bltu(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bltu(pc, (int32_t)operand1 - 10, operand1, branch_offset);

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bltu(pc, (int32_t)operand1 - 2, operand1, branch_offset);

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.branch.BLTU_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BLTU_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BLTU_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_bge(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BGE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BGE with lower value
  //    tick 1. Set inputs for BGE with equal value (core outputs result of BGE)
  //    tick 2. Set inputs for BGE with greater value (core outputs result of BGE)
  //    tick 3. Nothing (core outputs result of BGE)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = 2 + rand() % 6;
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_bge(pc, (int32_t)operand1 - 10, operand1, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bge(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bge(pc, (int32_t)operand1 + 10, operand1, branch_offset);

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.branch.BGE_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BGE_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BGE_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_bgeu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_BGEU;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for BLT with lower negative value
  //    tick 1. Set inputs for BLT with equal value (core outputs result of BLT)
  //    tick 2. Set inputs for BLT with greater positive value (core outputs result of BLT)
  //    tick 3. Set inputs for BLT with lower positive value (core outputs result of BLT)
  //    tick 4. Nothing (core outputs result of BLT)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = 2 + rand() % 6;
  uint32_t branch_offset = rand() % 0xFFFFF;
  tb->_bgeu(pc, (int32_t)operand1 - 10, operand1, branch_offset);

  //=================================
  //      Tick (0)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bgeu(pc, operand1, operand1, branch_offset);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bgeu(pc, (int32_t)operand1 + 10, operand1, branch_offset);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o   ==  0));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  pc + branch_offset));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_bgeu(pc, (int32_t)operand1 - 2, operand1, branch_offset);

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.branch.BGEU_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.BGEU_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.BGEU_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_back_to_back(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BACK_TO_BACK;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ADD
  //    tick 1. Set inputs for SUB (core outputs result of ADD)
  //    tick 2. Set inputs for ADD (core outputs result of SUB)
  //    tick 3. Nothing (core outputs result of ADD)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_add(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  operand1 = rand();
  operand2 = rand();
  reg_addr = rand() % 32;
  tb->_sub(operand1, operand2, reg_addr);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 - (int32_t)operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  operand1 = rand();
  operand2 = rand();
  reg_addr = rand() % 32;
  tb->_add(operand1, operand2, reg_addr);

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.back_to_back.01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.back_to_back.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.back_to_back.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_bubble(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BUBBLE;

  // The following actions are performed in this test :
  //    tick 0. Set random invalidated inputs
  //    tick 1. Nothing (core outputs bubble)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;
  core->output_ready_i = 1;
  
  core->alu_operand1_i = rand();
  core->alu_operand2_i = rand();
  core->alu_op_i = rand() % 7;
  core->alu_sub_i = rand() % 2;
  core->alu_shift_left_i = rand() % 2;
  core->alu_signed_shift_i = rand() % 2;
  core->reg_write_i = 1;
  core->reg_addr_i = rand() % 32;
  core->branch_cond_i = 1 + rand() % 6;
  core->branch_offset_i = rand() % 0xFFFFF;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.bubble.01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.bubble.02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.bubble.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_pipeline_stall_after_reset(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_PIPELINE_STALL_AFTER_RESET;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ADD with unready output
  //    tick 1. Nothing (core waits)
  //    tick 2. Ready output (core waits)
  //    tick 3. Set inputs for SUB (core outputs result of ADD)
  //    tick 4. Nothing (core outputs result of SUB)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready, (core->input_ready_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 0;

  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_add(operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  1));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_sub(operand1, operand2, reg_addr);

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 - (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  1));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.wait_after_reset.01",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o", tb->err_cycles[COND_input_ready]);
    
  CHECK("tb_exm.wait_after_reset.02",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.wait_after_reset.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.wait_after_reset.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_pipeline_stall(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_PIPELINE_STALL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for ADD
  //    tick 1. Unready output and set inputs for SUB (core holds output)
  //    tick 2. Nothing (core holds output)
  //    tick 3. Ready output (core outputs result of ADD)
  //    tick 4. Nothing (core outputs result of SUB)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;

  uint32_t operand1 = rand();
  uint32_t operand2 = rand();
  uint8_t reg_addr = rand() % 32;
  tb->_add(operand1, operand2, reg_addr);
  
  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  uint32_t result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  1));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 0;

  tb->_sub(operand1, operand2, reg_addr + 10);

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 + (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 1;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  result = ((int32_t)operand1 - (int32_t)operand2);
  tb->check(COND_input_ready,  (core->input_ready_o   ==  1));
  tb->check(COND_result,       (core->result_o        ==  result)  &&
                               (core->reg_write_o  ==  1)       &&
                               (core->reg_addr_o   ==  reg_addr + 10));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_exm.wait.01",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o", tb->err_cycles[COND_input_ready]);
    
  CHECK("tb_exm.wait.02",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.wait.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.wait.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_reset(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_RESET;

  // The following actions are performed in this test :
  //    tick 0. Set random inputs and reset
  //    tick 1. Unreset (output reset)
  //    tick 2. Set random inputs
  //    tick 3. Reset
  //    tick 4. Nothing (output reset)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;
  core->output_ready_i = 1;
  
  core->alu_operand1_i = rand();
  core->alu_operand2_i = core->alu_operand1_i;
  core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
  core->alu_shift_left_i = rand() % 2;
  core->alu_signed_shift_i = rand() % 2;
  core->reg_write_i = 1;
  core->reg_addr_i = rand() % 32;
  core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BEQ;
  core->branch_offset_i = rand() % 0xFFFFF;

  core->rst_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->rst_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;
  core->alu_operand1_i = rand();
  core->alu_operand2_i = core->alu_operand1_i;
  core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
  core->alu_shift_left_i = rand() % 2;
  core->alu_signed_shift_i = rand() % 2;
  core->reg_write_i = rand() % 2;
  core->reg_addr_i = rand() % 32;
  core->branch_cond_i = Vtb_exm_ecap5_dproc_pkg::BRANCH_BEQ;
  core->branch_offset_i = rand() % 0xFFFFF;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->rst_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,  (core->input_ready_o   ==  0));
  tb->check(COND_result,       (core->reg_write_o  ==  0));
  tb->check(COND_branch,       (core->branch_o        ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_exm.reset.01",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o", tb->err_cycles[COND_input_ready]);

  CHECK("tb_exm.reset.02",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.reset.03",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.reset.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

void tb_exm_branch_jalr(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  core->testcase = T_BRANCH_JALR;

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

  uint32_t pc = rand() % 0x7FFFFFFF;
  uint32_t operand1 = rand() % 0x7FFFFFFF;
  uint32_t operand2 = rand() % 0x7FFFFFFF;
  uint8_t reg_addr = rand() % 32;
  tb->_jalr(pc, operand1, operand2, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_result,       (core->result_o         ==  pc + 4)  &&
                               (core->reg_write_o   ==  1)       &&
                               (core->reg_addr_o    ==  reg_addr));
  tb->check(COND_branch,       (core->branch_o         ==  1) &&
                               (core->branch_target_o  ==  operand1 + operand2));
  tb->check(COND_output_valid, (core->output_valid_o   ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_exm.branch.JALR_01",
      tb->conditions[COND_result],
      "Failed to implement the result protocol", tb->err_cycles[COND_result]);

  CHECK("tb_exm.branch.JALR_02",
      tb->conditions[COND_branch],
      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);

  CHECK("tb_exm.branch.JALR_03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Exm * tb = new TB_Exm;
  tb->open_trace("waves/exm.vcd");
  tb->open_testdata("testdata/exm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_exm_alu_add(tb);
  tb_exm_alu_sub(tb);
  tb_exm_alu_xor(tb);
  tb_exm_alu_or(tb);
  tb_exm_alu_and(tb);
  tb_exm_alu_slt(tb);
  tb_exm_alu_sltu(tb);
  tb_exm_alu_sll(tb);
  tb_exm_alu_srl(tb);
  tb_exm_alu_sra(tb);

  tb_exm_branch_beq(tb);
  tb_exm_branch_bne(tb);
  tb_exm_branch_blt(tb);
  tb_exm_branch_bltu(tb);
  tb_exm_branch_bge(tb);
  tb_exm_branch_bgeu(tb);

  tb_exm_branch_jalr(tb);

  tb_exm_back_to_back(tb);
  tb_exm_bubble(tb);
  tb_exm_reset(tb);
  tb_exm_pipeline_stall_after_reset(tb);
  tb_exm_pipeline_stall(tb);

  /************************************************************/

  printf("[EXM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
