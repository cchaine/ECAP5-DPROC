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
    core->instr_i = 0;
    core->pc_i = 0;
  }

  void _lui(uint32_t pc, uint8_t rd, uint32_t imm20) {
    uint32_t instr = ((imm20 & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | 0b0110111;
    core->instr_i = instr;
    core->pc_i = pc;
  }
};

enum CondId {
  COND_input_ready,
  COND_result,
  COND_branch,
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
  uint8_t imm20 = rand() % 0xFFFFF;
  tb->_lui(pc, rd, imm20);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  

  //`````````````````````````````````
  //      Formal Checks 
  
//  CHECK("tb_decm.alu.ADD_01",
//      tb->conditions[COND_result],
//      "Failed to implement the result protocol", tb->err_cycles[COND_result]);
//
//  CHECK("tb_decm.alu.ADD_02",
//      tb->conditions[COND_branch],
//      "Failed to implement the branch protocol", tb->err_cycles[COND_branch]);
//
//  CHECK("tb_decm.alu.ADD_03",
//      tb->conditions[COND_output_valid],
//      "Failed to implement the output_valid_o", tb->err_cycles[COND_output_valid]);
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
