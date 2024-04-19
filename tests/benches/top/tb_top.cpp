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

  uint32_t instr_i(uint32_t opcode, uint32_t rd, uint32_t func3, uint32_t rs1, uint32_t imm) {
    uint32_t instr = 0;
    instr |= opcode & 0x7F;
    instr |= (rd & 0x1F) << 7;
    instr |= (func3 & 0x7) << 12;
    instr |= (rs1 & 0x1F) << 15;
    instr |= (imm & 0xFFF) << 20;
    return instr;
  }

  uint32_t _xori(uint32_t rd, uint32_t rs1, uint32_t imm) {
    return instr_i(Vtb_top_ecap5_dproc_pkg::OPCODE_OP_IMM, rd, Vtb_top_ecap5_dproc_pkg::FUNC3_XOR, rs1, imm);
  }

};

void tb_top_nop(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = 1;

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
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0. Nothing
  //    tick 1. Acknowledge request with XORI instruction and stall interface (core requests instruction)
  //    tick 2. Nothing (core ends request)
  //    tick 3. Nothing (decm processes instruction)
  //    tick 4. Nothing (exm processes instruction)
  //    tick 5. Nothing (lsm processes instruction)
  //    tick 6. Nothing (regm processes instruction)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t rd = rand() % 32;
  uint32_t rs1 = rand() % 32;
  uint32_t imm = rand() % 0xFFF;
  uint32_t instr = tb->_xori(rd, rs1, imm);
  core->wb_dat_i = instr;
  core->wb_ack_i = 1;

  core->wb_stall_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.alu.01",
      tb->conditions[COND_bubble],
      "Failed to implement the pipeline bubbles", tb->err_cycles[COND_bubble]);
}

void tb_top_lsm_enable(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = 2;

  tb->reset();
}

void tb_top_branch(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = 3;

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
