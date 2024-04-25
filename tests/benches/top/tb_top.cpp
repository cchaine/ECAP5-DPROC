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

#include "Vtb_top_ecap5_dproc_pkg.h"
#include "Vtb_top.h"
#include "Vtb_top_top.h"
#include "Vtb_top_tb_top.h"
#include "testbench.h"

enum CondId {
  COND_ready,
  COND_valid,
  COND_bubble,
  COND_ifm,
  COND_decm,
  COND_exm,
  COND_lsm,
  COND_wbm,
  __CondIdEnd
};

enum TestcaseId {
  T_NOP           =  1,
  T_ALU           =  2,
  T_LSM_ENABLE    =  3,
  T_BRANCH        =  4,
  T_BACK_TO_BACK  =  5
};

class TB_Top : public Testbench<Vtb_top> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_top>::reset();
  }

  void tick() {
    uint32_t prev_ifm_decm_valid = this->core->tb_top->dut->ifm_decm_valid;
    uint32_t prev_decm_exm_valid = this->core->tb_top->dut->decm_exm_valid;
    uint32_t prev_exm_lsm_valid = this->core->tb_top->dut->exm_lsm_valid;
    uint32_t prev_lsm_valid = this->core->tb_top->dut->lsm_valid;

    Testbench<Vtb_top>::tick();

    // Check pipeline bubbles
    if(prev_ifm_decm_valid == 0) {
      this->check(COND_bubble, (this->core->tb_top->dut->decm_alu_operand1 == 0)         &&
                                  (this->core->tb_top->dut->decm_alu_operand2 == 0)         &&
                                  (this->core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_ADD)   &&
                                  (this->core->tb_top->dut->decm_alu_sub      == 0)         &&
                                  (this->core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::NO_BRANCH) &&
                                  (this->core->tb_top->dut->decm_ls_enable    == 0)         &&
                                  (this->core->tb_top->dut->decm_reg_write    == 0));
    }
    if(prev_decm_exm_valid == 0) {
      this->check(COND_bubble, (this->core->tb_top->dut->exm_ls_enable  == 0) &&
                                 (this->core->tb_top->dut->exm_reg_write  == 0));
    }
    if(prev_exm_lsm_valid == 0) {
      this->check(COND_bubble, (this->core->tb_top->dut->lsm_reg_write  == 0));
    }
    if(prev_lsm_valid == 0) {
      this->check(COND_bubble, (this->core->tb_top->dut->reg_write      == 0));
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
    return instr_i(Vtb_top_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_top_ecap5_dproc_pkg::FUNC3_XOR, rs1, imm);
  }

  uint32_t _lb(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_top_ecap5_dproc_pkg::OPCODE_LOAD, rd, Vtb_top_ecap5_dproc_pkg::FUNC3_LB, rs1, imm);
  }

  uint32_t _addi(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_top_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_top_ecap5_dproc_pkg::FUNC3_ADD, rs1, imm);
  }

  uint32_t _beq(uint32_t rs1, uint32_t rs2, uint32_t imm) {
    return instr_b(Vtb_top_ecap5_dproc_pkg::OPCODE_BRANCH, Vtb_top_ecap5_dproc_pkg::FUNC3_BEQ, rs1, rs2, imm);
  }

  void set_register(uint8_t addr, uint32_t value) {
    const svScope scope = svGetScopeFromName("TOP.tb_top.dut.regm_inst");
    assert(scope);
    svSetScope(scope);
    this->core->set_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)&value); 
  }

};

void tb_top_nop(TB_Top * tb) {
  Vtb_top * core = tb->core;
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
  
  tb->check(COND_ready, (core->tb_top->dut->ifm_decm_ready == 1) &&
                        (core->tb_top->dut->decm_exm_ready == 1) &&
                        (core->tb_top->dut->exm_lsm_ready  == 1));

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->decm_exm_valid == 1) &&
                        (core->tb_top->dut->exm_lsm_valid  == 1) &&
                        (core->tb_top->dut->lsm_valid      == 1));
  tb->check(COND_decm, (core->tb_top->dut->decm_alu_operand1 == 0)         &&
                              (core->tb_top->dut->decm_alu_operand2 == 0)         &&
                              (core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_ADD)   &&
                              (core->tb_top->dut->decm_alu_sub      == 0)         &&
                              (core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::NO_BRANCH) &&
                              (core->tb_top->dut->decm_ls_enable    == 0)         &&
                              (core->tb_top->dut->decm_reg_write    == 0));
  tb->check(COND_exm, (core->tb_top->dut->exm_ls_enable  == 0) &&
                             (core->tb_top->dut->exm_reg_write  == 0));
  tb->check(COND_lsm, (core->tb_top->dut->lsm_reg_write  == 0));
  tb->check(COND_wbm, (core->tb_top->dut->reg_write      == 0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.nop.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);
  
  CHECK("tb_top.nop.02",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_top.nop.03",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_top.nop.04",
      tb->conditions[COND_decm],
      "Failed to implement the decm", tb->err_cycles[COND_decm]);

  CHECK("tb_top.nop.05",
      tb->conditions[COND_exm],
      "Failed to implement the exm", tb->err_cycles[COND_exm]);

  CHECK("tb_top.nop.06",
      tb->conditions[COND_lsm],
      "Failed to implement the lsm", tb->err_cycles[COND_lsm]);

  CHECK("tb_top.nop.07",
      tb->conditions[COND_wbm],
      "Failed to implement the wbm", tb->err_cycles[COND_wbm]);
}

void tb_top_alu(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = T_ALU;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with XORI instruction and stall interface (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (decm processes instruction)
  //    tick 4. Nothing (exm processes instruction)
  //    tick 5. Nothing (lsm processes instruction)
  //    tick 6. Nothing (wbm processes instruction)
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
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid == 1));
  tb->check(COND_ifm, (core->tb_top->dut->ifm_instr == instr) &&
                      (core->tb_top->dut->ifm_pc == Vtb_top_ecap5_dproc_pkg::BOOT_ADDRESS));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid   == 0));
  tb->check(COND_decm, (core->tb_top->dut->decm_pc           == Vtb_top_ecap5_dproc_pkg::BOOT_ADDRESS) &&
                       (core->tb_top->dut->decm_alu_operand1 == rs1_val)                               &&
                       (core->tb_top->dut->decm_alu_operand2 == tb->sign_extend(imm, 12))              &&
                       (core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_XOR)      &&
                       (core->tb_top->dut->decm_alu_sub      == 0)                                     &&
                       (core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::NO_BRANCH)    &&
                       (core->tb_top->dut->decm_ls_enable    == 0)                                     &&
                       (core->tb_top->dut->decm_reg_write    == 1)                                     &&
                       (core->tb_top->dut->decm_reg_addr     == rd));

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_exm, (core->tb_top->dut->exm_result    == result) &&
                      (core->tb_top->dut->exm_ls_enable == 0)      &&
                      (core->tb_top->dut->exm_reg_write == 1)      &&
                      (core->tb_top->dut->exm_reg_addr  == rd)     &&
                      (core->tb_top->dut->branch == 0));

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_lsm, (core->tb_top->dut->lsm_reg_write == 1) &&
                      (core->tb_top->dut->lsm_reg_addr == rd) &&
                      (core->tb_top->dut->lsm_reg_data == result));

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wbm, (core->tb_top->dut->reg_write == 1) &&
                      (core->tb_top->dut->reg_waddr == rd) &&
                      (core->tb_top->dut->reg_wdata == result));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.alu.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_top.alu.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_top.alu.03",
      tb->conditions[COND_ifm],
      "Failed to implement the ifm", tb->err_cycles[COND_ifm]);

  CHECK("tb_top.alu.04",
      tb->conditions[COND_decm],
      "Failed to implement the decm", tb->err_cycles[COND_decm]);

  CHECK("tb_top.alu.05",
      tb->conditions[COND_exm],
      "Failed to implement the exm", tb->err_cycles[COND_exm]);

  CHECK("tb_top.alu.06",
      tb->conditions[COND_lsm],
      "Failed to implement the lsm", tb->err_cycles[COND_lsm]);

  CHECK("tb_top.alu.07",
      tb->conditions[COND_wbm],
      "Failed to implement the wbm", tb->err_cycles[COND_wbm]);
}

void tb_top_lsm_enable(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = T_LSM_ENABLE;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with LB instruction (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (decm processes instruction)
  //    tick 4. Nothing (exm processes instruction)
  //    tick 5. Acknowledge instr request with nop (lsm processes instruction and makes request)
  //    tick 6. Nothing (lsm holds request)
  //    tick 7. Nothing (lsm holds request)
  //    tick 8. Nothing (lsm holds request)
  //    tick 9. Nothing (lsm holds request)
  //    tick 10. Acknowledge lsm request (lsm makes request)
  //    tick 11. Nothing (lsm latches response)
  //    tick 12. Nothing (wbm processes instruction)
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
  
  tb->check(COND_ifm, (core->tb_top->dut->ifm_instr == instr));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_decm, (core->tb_top->dut->decm_alu_operand1 == rs1_val) &&
                       (core->tb_top->dut->decm_alu_operand2 == tb->sign_extend(imm, 12)) &&
                       (core->tb_top->dut->decm_alu_op == Vtb_top_ecap5_dproc_pkg::ALU_ADD) &&
                       (core->tb_top->dut->decm_alu_sub == 0) &&
                       (core->tb_top->dut->decm_ls_enable == 1) &&
                       (core->tb_top->dut->decm_ls_write == 0) &&
                       (core->tb_top->dut->decm_ls_sel == 1) &&
                       (core->tb_top->dut->decm_ls_unsigned_load == 0) &&
                       (core->tb_top->dut->decm_reg_write == 1) &&
                       (core->tb_top->dut->decm_reg_addr == rd));

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 1));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 1));
  tb->check(COND_exm, (core->tb_top->dut->exm_result == rs1_val + tb->sign_extend(imm, 12)) &&
                      (core->tb_top->dut->exm_ls_enable == 1) &&
                      (core->tb_top->dut->exm_ls_write == 0) &&
                      (core->tb_top->dut->exm_ls_unsigned_load == 0) &&
                      (core->tb_top->dut->exm_reg_write == 1) &&
                      (core->tb_top->dut->exm_reg_addr == rd) &&
                      (core->tb_top->dut->branch == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = tb->_addi(0, 0, 0);
  core->wb_ack_i = 1;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 0));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 0));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 0));

  //=================================
  //      Tick (8)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 0));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 0));

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
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 0));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (10)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_ready, (core->tb_top->dut->exm_lsm_ready == 1));
  tb->check(COND_valid, (core->tb_top->dut->lsm_valid == 1));
  tb->check(COND_lsm, (core->tb_top->dut->lsm_reg_write == 1) &&
                      (core->tb_top->dut->lsm_reg_addr == rd) &&
                      (core->tb_top->dut->lsm_reg_data == tb->sign_extend(lb_data & 0xFF, 8)));

  //=================================
  //      Tick (11)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wbm, (core->tb_top->dut->reg_write == 1) &&
                      (core->tb_top->dut->reg_waddr == rd) &&
                      (core->tb_top->dut->reg_wdata == tb->sign_extend(lb_data & 0xFF, 8)));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.lsm_enable.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_top.lsm_enable.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_top.lsm_enable.03",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_top.lsm_enable.04",
      tb->conditions[COND_ifm],
      "Failed to implement the ifm", tb->err_cycles[COND_ifm]);

  CHECK("tb_top.lsm_enable.05",
      tb->conditions[COND_decm],
      "Failed to implement the decm", tb->err_cycles[COND_decm]);

  CHECK("tb_top.lsm_enable.06",
      tb->conditions[COND_exm],
      "Failed to implement the exm", tb->err_cycles[COND_exm]);

  CHECK("tb_top.lsm_enable.07",
      tb->conditions[COND_lsm],
      "Failed to implement the lsm", tb->err_cycles[COND_lsm]);

  CHECK("tb_top.lsm_enable.08",
      tb->conditions[COND_wbm],
      "Failed to implement the wbm", tb->err_cycles[COND_wbm]);
}

void tb_top_branch(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = T_BRANCH;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with BEQ instruction (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (decm processes instruction)
  //    tick 4. Acknowledge instr2 with nop (exm processes instruction)
  //    tick 5. Nothing (ifm requests branch target)

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
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid == 1));
  tb->check(COND_ifm, (core->tb_top->dut->ifm_instr == instr) &&
                      (core->tb_top->dut->ifm_pc == Vtb_top_ecap5_dproc_pkg::BOOT_ADDRESS));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->ifm_decm_valid   == 0));
  tb->check(COND_decm, (core->tb_top->dut->decm_pc           == Vtb_top_ecap5_dproc_pkg::BOOT_ADDRESS) &&
                       (core->tb_top->dut->decm_alu_operand1 == val)                                   &&
                       (core->tb_top->dut->decm_alu_operand2 == val)                                   &&
                       (core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_ADD)      &&
                       (core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::BRANCH_BEQ)   &&
                       (core->tb_top->dut->decm_ls_enable    == 0)                                     &&
                       (core->tb_top->dut->decm_reg_write    == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = tb->_addi(0, 0, 0);
  core->wb_ack_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_exm, (core->tb_top->dut->exm_ls_enable == 0)      &&
                      (core->tb_top->dut->exm_reg_write == 0) &&
                      (core->tb_top->dut->branch == 1) &&
                      (core->tb_top->dut->branch_target == tb->sign_extend(imm, 13)));

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
  //      Checks 
  
  tb->check(COND_ifm, (core->wb_adr_o == tb->sign_extend(imm, 13)));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.branch.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);

  CHECK("tb_top.branch.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_top.branch.03",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_top.branch.04",
      tb->conditions[COND_ifm],
      "Failed to implement the ifm", tb->err_cycles[COND_ifm]);

  CHECK("tb_top.branch.05",
      tb->conditions[COND_decm],
      "Failed to implement the decm", tb->err_cycles[COND_decm]);

  CHECK("tb_top.branch.06",
      tb->conditions[COND_exm],
      "Failed to implement the exm", tb->err_cycles[COND_exm]);

  CHECK("tb_top.branch.07",
      tb->conditions[COND_lsm],
      "Failed to implement the lsm", tb->err_cycles[COND_lsm]);

  CHECK("tb_top.branch.08",
      tb->conditions[COND_wbm],
      "Failed to implement the wbm", tb->err_cycles[COND_wbm]);
}

void tb_top_back_to_back(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = T_BACK_TO_BACK;

  tb->reset();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Top * tb = new TB_Top();
  tb->open_trace("waves/top.vcd");
  tb->open_testdata("testdata/top.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_top_nop(tb);
  tb_top_alu(tb);
  tb_top_lsm_enable(tb);
  tb_top_branch(tb);

  tb_top_back_to_back(tb);

  /************************************************************/

  printf("[TOP]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
