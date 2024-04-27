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

#include "Vtb_ecap5_dproc_ecap5_dproc_pkg.h"
#include "Vtb_ecap5_dproc.h"
#include "Vtb_ecap5_dproc_ecap5_dproc.h"
#include "Vtb_ecap5_dproc_tb_ecap5_dproc.h"
#include "testbench.h"

enum CondId {
  COND_ready,
  COND_valid,
  COND_bubble,
  COND_fetch,
  COND_decode,
  COND_execute,
  COND_loadstore,
  COND_writeback,
  COND_hazard,
  __CondIdEnd
};

enum TestcaseId {
  T_NOP           =  1,
  T_ALU           =  2,
  T_LSM_ENABLE    =  3,
  T_BRANCH        =  4,
  T_BACK_TO_BACK  =  5,
  T_DATA_HAZARD   =  6
};

class TB_Ecap5_dproc : public Testbench<Vtb_ecap5_dproc> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_ecap5_dproc>::reset();
  }

  void tick() {
    uint32_t prev_if_dec_valid = this->core->tb_ecap5_dproc->dut->if_dec_valid;
    uint32_t prev_dec_ex_valid = this->core->tb_ecap5_dproc->dut->dec_ex_valid;
    uint32_t prev_ex_ls_valid = this->core->tb_ecap5_dproc->dut->ex_ls_valid;
    uint32_t prev_ls_valid = this->core->tb_ecap5_dproc->dut->ls_valid;

    Testbench<Vtb_ecap5_dproc>::tick();

    // Check pipeline bubbles
    if(prev_if_dec_valid == 0) {
      this->check(COND_bubble, (this->core->tb_ecap5_dproc->dut->dec_alu_operand1 == 0)         &&
                                  (this->core->tb_ecap5_dproc->dut->dec_alu_operand2 == 0)         &&
                                  (this->core->tb_ecap5_dproc->dut->dec_alu_op       == Vtb_ecap5_dproc_ecap5_dproc_pkg::ALU_ADD)   &&
                                  (this->core->tb_ecap5_dproc->dut->dec_alu_sub      == 0)         &&
                                  (this->core->tb_ecap5_dproc->dut->dec_branch_cond  == Vtb_ecap5_dproc_ecap5_dproc_pkg::NO_BRANCH) &&
                                  (this->core->tb_ecap5_dproc->dut->dec_ls_enable    == 0)         &&
                                  (this->core->tb_ecap5_dproc->dut->dec_reg_write    == 0));
    }
    if(prev_dec_ex_valid == 0) {
      this->check(COND_bubble, (this->core->tb_ecap5_dproc->dut->ex_ls_enable  == 0) &&
                                 (this->core->tb_ecap5_dproc->dut->ex_reg_write  == 0));
    }
    if(prev_ex_ls_valid == 0) {
      this->check(COND_bubble, (this->core->tb_ecap5_dproc->dut->ls_reg_write  == 0));
    }
    if(prev_ls_valid == 0) {
      this->check(COND_bubble, (this->core->tb_ecap5_dproc->dut->reg_write      == 0));
    }
  }

  void _nop() {
    this->core->irq_i = 0;
    this->core->drq_i = 0;
    this->core->wb_dat_i = 0;
    this->core->wb_ack_i = 0;
    this->core->wb_stall_i = 0;
  }

  uint32_t sign_extend(uint32_t data, uint32_t nb_bits) {
    data &= (1 << nb_bits)-1;
    if((data >> (nb_bits-1)) & 0x1){
      data |= (((1 << (32 - (nb_bits-1))) - 1) << nb_bits);
    }
    return data;
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

  uint32_t _xori(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_ecap5_dproc_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_ecap5_dproc_ecap5_dproc_pkg::FUNC3_XOR, rs1, imm);
  }

  uint32_t _lb(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_ecap5_dproc_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_ecap5_dproc_ecap5_dproc_pkg::FUNC3_LB, rs1, imm);
  }

  uint32_t _addi(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_ecap5_dproc_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_ecap5_dproc_ecap5_dproc_pkg::FUNC3_ADD, rs1, imm);
  }

  uint32_t _beq(uint32_t rs1, uint32_t rs2, uint32_t imm) {
    return instr_b(Vtb_ecap5_dproc_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_ecap5_dproc_ecap5_dproc_pkg::FUNC3_BEQ, rs1, rs2, imm);
  }

  void set_register(uint8_t addr, uint32_t value) {
    const svScope scope = svGetScopeFromName("TOP.tb_ecap5_dproc.dut.regs_inst");
    assert(scope);
    svSetScope(scope);
    this->core->set_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)&value); 
  }

};

void tb_ecap5_dproc_nop(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_NOP;

  // The following actions are performed in this test :
  //    tick 0. Stall the memory interface
  //    tick 1. Nothing
  //    tick 2. Nothing

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs

  core->wb_stall_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->if_dec_ready == 1) &&
                        (core->tb_ecap5_dproc->dut->dec_ex_ready == 1) &&
                        (core->tb_ecap5_dproc->dut->ex_ls_ready  == 1));

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->dec_ex_valid == 1) &&
                        (core->tb_ecap5_dproc->dut->ex_ls_valid  == 1) &&
                        (core->tb_ecap5_dproc->dut->ls_valid      == 1));
  tb->check(COND_decode, (core->tb_ecap5_dproc->dut->dec_alu_operand1 == 0)         &&
                              (core->tb_ecap5_dproc->dut->dec_alu_operand2 == 0)         &&
                              (core->tb_ecap5_dproc->dut->dec_alu_op       == Vtb_ecap5_dproc_ecap5_dproc_pkg::ALU_ADD)   &&
                              (core->tb_ecap5_dproc->dut->dec_alu_sub      == 0)         &&
                              (core->tb_ecap5_dproc->dut->dec_branch_cond  == Vtb_ecap5_dproc_ecap5_dproc_pkg::NO_BRANCH) &&
                              (core->tb_ecap5_dproc->dut->dec_ls_enable    == 0)         &&
                              (core->tb_ecap5_dproc->dut->dec_reg_write    == 0));
  tb->check(COND_execute, (core->tb_ecap5_dproc->dut->ex_ls_enable  == 0) &&
                             (core->tb_ecap5_dproc->dut->ex_reg_write  == 0));
  tb->check(COND_loadstore, (core->tb_ecap5_dproc->dut->ls_reg_write  == 0));
  tb->check(COND_writeback, (core->tb_ecap5_dproc->dut->reg_write      == 0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ecap5_dproc.nop.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);
  
  CHECK("tb_ecap5_dproc.nop.02",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_ecap5_dproc.nop.03",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_ecap5_dproc.nop.04",
      tb->conditions[COND_decode],
      "Failed to implement the decode stage", tb->err_cycles[COND_decode]);

  CHECK("tb_ecap5_dproc.nop.05",
      tb->conditions[COND_execute],
      "Failed to implement the execute stage", tb->err_cycles[COND_execute]);

  CHECK("tb_ecap5_dproc.nop.06",
      tb->conditions[COND_loadstore],
      "Failed to implement the loadstore stage", tb->err_cycles[COND_loadstore]);

  CHECK("tb_ecap5_dproc.nop.07",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback stage", tb->err_cycles[COND_writeback]);
}

void tb_ecap5_dproc_alu(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_ALU;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with XORI instruction and stall interface (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (dec processes instruction)
  //    tick 4. Nothing (ex processes instruction)
  //    tick 5. Nothing (ls processes instruction)
  //    tick 6. Nothing (wb processes instruction)
  //    tick 7. Nothing (intruction processed)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs1_val = rand();
  // Artificially set the source register value
  tb->set_register(rs1, rs1_val);
  uint32_t imm = rand() % 0xFFF;
  uint32_t instr = tb->_xori(rd, rs1, imm);
  uint32_t result = rs1_val ^ tb->sign_extend(imm, 12);

  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  core->wb_stall_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid == 1));
  tb->check(COND_fetch, (core->tb_ecap5_dproc->dut->if_instr == instr) &&
                      (core->tb_ecap5_dproc->dut->if_pc == Vtb_ecap5_dproc_ecap5_dproc_pkg::BOOT_ADDRESS));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid   == 0));
  tb->check(COND_decode, (core->tb_ecap5_dproc->dut->dec_pc           == Vtb_ecap5_dproc_ecap5_dproc_pkg::BOOT_ADDRESS) &&
                       (core->tb_ecap5_dproc->dut->dec_alu_operand1 == rs1_val)                               &&
                       (core->tb_ecap5_dproc->dut->dec_alu_operand2 == tb->sign_extend(imm, 12))              &&
                       (core->tb_ecap5_dproc->dut->dec_alu_op       == Vtb_ecap5_dproc_ecap5_dproc_pkg::ALU_XOR)      &&
                       (core->tb_ecap5_dproc->dut->dec_alu_sub      == 0)                                     &&
                       (core->tb_ecap5_dproc->dut->dec_branch_cond  == Vtb_ecap5_dproc_ecap5_dproc_pkg::NO_BRANCH)    &&
                       (core->tb_ecap5_dproc->dut->dec_ls_enable    == 0)                                     &&
                       (core->tb_ecap5_dproc->dut->dec_reg_write    == 1)                                     &&
                       (core->tb_ecap5_dproc->dut->dec_reg_addr     == rd));

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_execute, (core->tb_ecap5_dproc->dut->ex_result    == result) &&
                      (core->tb_ecap5_dproc->dut->ex_ls_enable == 0)      &&
                      (core->tb_ecap5_dproc->dut->ex_reg_write == 1)      &&
                      (core->tb_ecap5_dproc->dut->ex_reg_addr  == rd)     &&
                      (core->tb_ecap5_dproc->dut->branch == 0));

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_loadstore, (core->tb_ecap5_dproc->dut->ls_reg_write == 1) &&
                      (core->tb_ecap5_dproc->dut->ls_reg_addr == rd) &&
                      (core->tb_ecap5_dproc->dut->ls_reg_data == result));

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_writeback, (core->tb_ecap5_dproc->dut->reg_write == 1) &&
                      (core->tb_ecap5_dproc->dut->reg_waddr == rd) &&
                      (core->tb_ecap5_dproc->dut->reg_wdata == result));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ecap5_dproc.alu.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_ecap5_dproc.alu.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_ecap5_dproc.alu.03",
      tb->conditions[COND_fetch],
      "Failed to implement the if", tb->err_cycles[COND_fetch]);

  CHECK("tb_ecap5_dproc.alu.04",
      tb->conditions[COND_decode],
      "Failed to implement the decode stage", tb->err_cycles[COND_decode]);

  CHECK("tb_ecap5_dproc.alu.05",
      tb->conditions[COND_execute],
      "Failed to implement the execute stage", tb->err_cycles[COND_execute]);

  CHECK("tb_ecap5_dproc.alu.06",
      tb->conditions[COND_loadstore],
      "Failed to implement the loadstore stage", tb->err_cycles[COND_loadstore]);

  CHECK("tb_ecap5_dproc.alu.07",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback stage", tb->err_cycles[COND_writeback]);
}

void tb_ecap5_dproc_ls_enable(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_LSM_ENABLE;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with LB instruction (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (dec processes instruction)
  //    tick 4. Nothing (ex processes instruction)
  //    tick 5. Acknowledge instr request with nop (ls processes instruction and makes request)
  //    tick 6. Nothing (ls holds request)
  //    tick 7. Nothing (ls holds request)
  //    tick 8. Nothing (ls holds request)
  //    tick 9. Nothing (ls holds request)
  //    tick 10. Acknowledge ls request (ls makes request)
  //    tick 11. Nothing (ls latches response)
  //    tick 12. Nothing (wb processes instruction)
  //    tick 13. Nothing (instruction processed)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t rs1_val = rand();
  // Artificially set the source register value
  tb->set_register(rs1, rs1_val);
  uint32_t imm = rand() % 0xFFF;
  uint32_t instr = tb->_lb(rd, rs1, imm);

  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_fetch, (core->tb_ecap5_dproc->dut->if_instr == instr));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_decode, (core->tb_ecap5_dproc->dut->dec_alu_operand1 == rs1_val) &&
                       (core->tb_ecap5_dproc->dut->dec_alu_operand2 == tb->sign_extend(imm, 12)) &&
                       (core->tb_ecap5_dproc->dut->dec_alu_op == Vtb_ecap5_dproc_ecap5_dproc_pkg::ALU_ADD) &&
                       (core->tb_ecap5_dproc->dut->dec_alu_sub == 0) &&
                       (core->tb_ecap5_dproc->dut->dec_ls_enable == 1) &&
                       (core->tb_ecap5_dproc->dut->dec_ls_write == 0) &&
                       (core->tb_ecap5_dproc->dut->dec_ls_sel == 1) &&
                       (core->tb_ecap5_dproc->dut->dec_ls_unsigned_load == 0) &&
                       (core->tb_ecap5_dproc->dut->dec_reg_write == 1) &&
                       (core->tb_ecap5_dproc->dut->dec_reg_addr == rd));

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 1));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 1));
  tb->check(COND_execute, (core->tb_ecap5_dproc->dut->ex_result == rs1_val + tb->sign_extend(imm, 12)) &&
                      (core->tb_ecap5_dproc->dut->ex_ls_enable == 1) &&
                      (core->tb_ecap5_dproc->dut->ex_ls_write == 0) &&
                      (core->tb_ecap5_dproc->dut->ex_ls_unsigned_load == 0) &&
                      (core->tb_ecap5_dproc->dut->ex_reg_write == 1) &&
                      (core->tb_ecap5_dproc->dut->ex_reg_addr == rd) &&
                      (core->tb_ecap5_dproc->dut->branch == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = tb->_addi(0, 0, 0);
  core->wb_ack_i = 1;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 0));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 0));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 0));

  //=================================
  //      Tick (8)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 0));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t lb_data = rand();
  core->wb_dat_i = lb_data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (9)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 0));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (10)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_ecap5_dproc->dut->ex_ls_ready == 1));
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->ls_valid == 1));
  tb->check(COND_loadstore, (core->tb_ecap5_dproc->dut->ls_reg_write == 1) &&
                      (core->tb_ecap5_dproc->dut->ls_reg_addr == rd) &&
                      (core->tb_ecap5_dproc->dut->ls_reg_data == tb->sign_extend(lb_data & 0xFF, 8)));

  //=================================
  //      Tick (11)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_writeback, (core->tb_ecap5_dproc->dut->reg_write == 1) &&
                      (core->tb_ecap5_dproc->dut->reg_waddr == rd) &&
                      (core->tb_ecap5_dproc->dut->reg_wdata == tb->sign_extend(lb_data & 0xFF, 8)));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ecap5_dproc.ls_enable.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_ecap5_dproc.ls_enable.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_ecap5_dproc.ls_enable.03",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_ecap5_dproc.ls_enable.04",
      tb->conditions[COND_fetch],
      "Failed to implement the if", tb->err_cycles[COND_fetch]);

  CHECK("tb_ecap5_dproc.ls_enable.05",
      tb->conditions[COND_decode],
      "Failed to implement the decode stage", tb->err_cycles[COND_decode]);

  CHECK("tb_ecap5_dproc.ls_enable.06",
      tb->conditions[COND_execute],
      "Failed to implement the execute stage", tb->err_cycles[COND_execute]);

  CHECK("tb_ecap5_dproc.ls_enable.07",
      tb->conditions[COND_loadstore],
      "Failed to implement the loadstore stage", tb->err_cycles[COND_loadstore]);

  CHECK("tb_ecap5_dproc.ls_enable.08",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback stage", tb->err_cycles[COND_writeback]);
}

void tb_ecap5_dproc_branch(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_BRANCH;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with BEQ instruction (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (dec processes instruction)
  //    tick 4. Acknowledge instr2 with nop (ex processes instruction)
  //    tick 5. Nothing (if requests branch target)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t rs1 = rand() % 32;
  uint32_t val = rand();
  // Artificially set the first source register value
  tb->set_register(rs1, val);
  uint32_t rs2 = rand() % 32;
  // Artificially set the second source register value
  tb->set_register(rs2, val);
  uint32_t imm = (rand() % 0x1FFF) & ~(0x1);
  uint32_t instr = tb->_beq(rs1, rs2, imm);

  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid == 1));
  tb->check(COND_fetch, (core->tb_ecap5_dproc->dut->if_instr == instr) &&
                      (core->tb_ecap5_dproc->dut->if_pc == Vtb_ecap5_dproc_ecap5_dproc_pkg::BOOT_ADDRESS));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_ecap5_dproc->dut->if_dec_valid   == 0));
  tb->check(COND_decode, (core->tb_ecap5_dproc->dut->dec_pc           == Vtb_ecap5_dproc_ecap5_dproc_pkg::BOOT_ADDRESS) &&
                       (core->tb_ecap5_dproc->dut->dec_alu_operand1 == val)                                   &&
                       (core->tb_ecap5_dproc->dut->dec_alu_operand2 == val)                                   &&
                       (core->tb_ecap5_dproc->dut->dec_alu_op       == Vtb_ecap5_dproc_ecap5_dproc_pkg::ALU_ADD)      &&
                       (core->tb_ecap5_dproc->dut->dec_branch_cond  == Vtb_ecap5_dproc_ecap5_dproc_pkg::BRANCH_BEQ)   &&
                       (core->tb_ecap5_dproc->dut->dec_ls_enable    == 0)                                     &&
                       (core->tb_ecap5_dproc->dut->dec_reg_write    == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = tb->_addi(0, 0, 0);
  core->wb_ack_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_execute, (core->tb_ecap5_dproc->dut->ex_ls_enable == 0)      &&
                      (core->tb_ecap5_dproc->dut->ex_reg_write == 0) &&
                      (core->tb_ecap5_dproc->dut->branch == 1) &&
                      (core->tb_ecap5_dproc->dut->branch_target == tb->sign_extend(imm, 13)));
  tb->check(COND_hazard, (core->tb_ecap5_dproc->dut->hzd_ex_discard_request == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_hazard, (core->tb_ecap5_dproc->dut->hzd_ex_discard_request == 1));

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_fetch, (core->wb_adr_o == tb->sign_extend(imm, 13)));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ecap5_dproc.branch.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_ecap5_dproc.branch.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_ecap5_dproc.branch.03",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_ecap5_dproc.branch.04",
      tb->conditions[COND_fetch],
      "Failed to implement the if", tb->err_cycles[COND_fetch]);

  CHECK("tb_ecap5_dproc.branch.05",
      tb->conditions[COND_decode],
      "Failed to implement the decode stage", tb->err_cycles[COND_decode]);

  CHECK("tb_ecap5_dproc.branch.06",
      tb->conditions[COND_execute],
      "Failed to implement the execute stage", tb->err_cycles[COND_execute]);

  CHECK("tb_ecap5_dproc.branch.07",
      tb->conditions[COND_loadstore],
      "Failed to implement the loadstore stage", tb->err_cycles[COND_loadstore]);

  CHECK("tb_ecap5_dproc.branch.08",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback stage", tb->err_cycles[COND_writeback]);

  CHECK("tb_ecap5_dproc.branch.09",
      tb->conditions[COND_hazard],
      "Failed to implement the hazard module", tb->err_cycles[COND_hazard]);
}

void tb_ecap5_dproc_data_hazard(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_DATA_HAZARD;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with ADDI x1, x0, 1 instruction (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (dec processes instruction)
  //    tick 4. Acknowledge request with ADDI x2, x1, 1 (ex processes instruction)
  //    tick 5. Nothing (ls processes instruction)
  //    tick 6. Nothing (wb processes instruction)
  //    tick 7. Nothing (intruction processed)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t instr = tb->_addi(1, 0, 1);

  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  instr = tb->_addi(2, 1, 1);

  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ecap5_dproc.data_hazard.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_ecap5_dproc.data_hazard.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_ecap5_dproc.data_hazard.03",
      tb->conditions[COND_fetch],
      "Failed to implement the if", tb->err_cycles[COND_fetch]);

  CHECK("tb_ecap5_dproc.data_hazard.04",
      tb->conditions[COND_decode],
      "Failed to implement the decode stage", tb->err_cycles[COND_decode]);

  CHECK("tb_ecap5_dproc.data_hazard.05",
      tb->conditions[COND_execute],
      "Failed to implement the execute stage", tb->err_cycles[COND_execute]);

  CHECK("tb_ecap5_dproc.data_hazard.06",
      tb->conditions[COND_loadstore],
      "Failed to implement the loadstore stage", tb->err_cycles[COND_loadstore]);

  CHECK("tb_ecap5_dproc.data_hazard.07",
      tb->conditions[COND_writeback],
      "Failed to implement the writeback stage", tb->err_cycles[COND_writeback]);
}

void tb_ecap5_dproc_back_to_back(TB_Ecap5_dproc * tb) {
  Vtb_ecap5_dproc * core = tb->core;
  core->testcase = T_BACK_TO_BACK;

  tb->reset();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Ecap5_dproc * tb = new TB_Ecap5_dproc();
  tb->open_trace("waves/ecap5_dproc.vcd");
  tb->open_testdata("testdata/ecap5_dproc.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_ecap5_dproc_nop(tb);
  tb_ecap5_dproc_alu(tb);
  tb_ecap5_dproc_ls_enable(tb);
  tb_ecap5_dproc_branch(tb);

  tb_ecap5_dproc_data_hazard(tb);

  tb_ecap5_dproc_back_to_back(tb);

  /************************************************************/

  printf("[ECAP5_DPROC]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
