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

class TB_Top : public Testbench<Vtb_top> {
public:
  void reset() {
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_top>::reset();
  }
};

enum CondId {
  COND_ready,
  COND_valid,
  COND_decm_bubble,
  COND_exm_bubble,
  COND_lsm_bubble,
  COND_wbm_bubble,
  __CondIdEnd
};

void tb_top_nop(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0. Stall the memory interface
  //    tick 1. Nothing

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
  tb->check(COND_decm_bubble, (core->tb_top->dut->decm_alu_operand1 == 0)         &&
                              (core->tb_top->dut->decm_alu_operand2 == 0)         &&
                              (core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_ADD)   &&
                              (core->tb_top->dut->decm_alu_sub      == 0)         &&
                              (core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::NO_BRANCH) &&
                              (core->tb_top->dut->decm_ls_enable    == 0)         &&
                              (core->tb_top->dut->decm_reg_write    == 0));
  tb->check(COND_exm_bubble, (core->tb_top->dut->exm_ls_enable  == 0) &&
                             (core->tb_top->dut->exm_reg_write  == 0));
  tb->check(COND_lsm_bubble, (core->tb_top->dut->lsm_reg_write  == 0));
  tb->check(COND_wbm_bubble, (core->tb_top->dut->reg_write      == 0));

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_valid, (core->tb_top->dut->decm_exm_valid == 1) &&
                        (core->tb_top->dut->exm_lsm_valid  == 1) &&
                        (core->tb_top->dut->lsm_valid      == 1));
  tb->check(COND_decm_bubble, (core->tb_top->dut->decm_alu_operand1 == 0)         &&
                              (core->tb_top->dut->decm_alu_operand2 == 0)         &&
                              (core->tb_top->dut->decm_alu_op       == Vtb_top_ecap5_dproc_pkg::ALU_ADD)   &&
                              (core->tb_top->dut->decm_alu_sub      == 0)         &&
                              (core->tb_top->dut->decm_branch_cond  == Vtb_top_ecap5_dproc_pkg::NO_BRANCH) &&
                              (core->tb_top->dut->decm_ls_enable    == 0)         &&
                              (core->tb_top->dut->decm_reg_write    == 0));
  tb->check(COND_exm_bubble, (core->tb_top->dut->exm_ls_enable  == 0) &&
                             (core->tb_top->dut->exm_reg_write  == 0));
  tb->check(COND_lsm_bubble, (core->tb_top->dut->lsm_reg_write  == 0));
  tb->check(COND_wbm_bubble, (core->tb_top->dut->reg_write      == 0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_top.nop.01",
      tb->conditions[COND_ready],
      "Failed to implement the ready signal", tb->err_cycles[COND_ready]);

  CHECK("tb_top.nop.02",
      tb->conditions[COND_valid],
      "Failed to implement the valid signal", tb->err_cycles[COND_valid]);

  CHECK("tb_top.nop.03",
      tb->conditions[COND_decm_bubble],
      "Failed to implement the decm bubble", tb->err_cycles[COND_decm_bubble]);

  CHECK("tb_top.nop.04",
      tb->conditions[COND_exm_bubble],
      "Failed to implement the exm bubble", tb->err_cycles[COND_exm_bubble]);

  CHECK("tb_top.nop.05",
      tb->conditions[COND_lsm_bubble],
      "Failed to implement the lsm bubble", tb->err_cycles[COND_lsm_bubble]);

  CHECK("tb_top.nop.06",
      tb->conditions[COND_wbm_bubble],
      "Failed to implement the wbm bubble", tb->err_cycles[COND_wbm_bubble]);
}

void tb_top_alu(TB_Top * tb) {
  Vtb_top * core = tb->core;
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for LUI 
  //    tick 1. Nothing (core outputs result of LUI)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //=================================
  //      Tick (1)
  
  tb->tick();
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
