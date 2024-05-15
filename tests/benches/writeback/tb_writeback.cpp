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

#include "Vtb_writeback.h"
#include "testbench.h"
#include "Vtb_writeback_ecap5_dproc_pkg.h"

enum CondId {
  COND_output,
  __CondIdEnd
};

enum TestcaseId {
  T_WRITE = 1,
  T_BYPASS = 2,
  T_BUBBLE = 3,
  T_RESET = 4
};

class TB_Writeback : public Testbench<Vtb_writeback> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_writeback>::reset();
  }
  
  void _nop() {
    core->reg_write_i = 0;
    core->reg_addr_i = 0;
    core->reg_data_i = 0;
  }
};

void tb_writeback_reset(TB_Writeback * tb) {
  Vtb_writeback * core = tb->core;
  core->testcase = T_RESET;

  tb->reset();

  CHECK("tb_writeback.reset.01",
      false,
      "TODO");
}

void tb_writeback_write(TB_Writeback * tb) {
  Vtb_writeback * core = tb->core;
  core->testcase = T_WRITE;

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;

  core->reg_write_i = 1;
  uint32_t addr = rand() % 32;
  core->reg_addr_i = addr;
  uint32_t data = rand();
  core->reg_data_i = data;

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_output, (core->reg_write_o == 1) &&
                         (core->reg_addr_o == addr) &&
                         (core->reg_data_o == data));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_writeback.write.01",
      tb->conditions[COND_output],
      "Failed to implement writeback module", tb->err_cycles[COND_output]);
}

void tb_writeback_bypass(TB_Writeback * tb) {
  Vtb_writeback * core = tb->core;
  core->testcase = T_BYPASS;

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;

  core->reg_write_i = 0;
  uint32_t addr = rand() % 32;
  core->reg_addr_i = addr;
  uint32_t data = rand();
  core->reg_data_i = data;

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_output, (core->reg_write_o == 0));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_writeback.bypass.01",
      tb->conditions[COND_output],
      "Failed to implement writeback module", tb->err_cycles[COND_output]);
}

void tb_writeback_bubble(TB_Writeback * tb) {
  Vtb_writeback * core = tb->core;
  core->testcase = T_BUBBLE;

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;

  core->reg_write_i = 1;
  uint32_t addr = rand() % 32;
  core->reg_addr_i = addr;
  uint32_t data = rand();
  core->reg_data_i = data;

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_output, (core->reg_write_o == 0));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_writeback.bubble.01",
      tb->conditions[COND_output],
      "Failed to implement writeback module", tb->err_cycles[COND_output]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Writeback * tb = new TB_Writeback;
  tb->open_trace("waves/writeback.vcd");
  tb->open_testdata("testdata/writeback.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_writeback_reset(tb);

  tb_writeback_write(tb);

  tb_writeback_bypass(tb);

  tb_writeback_bubble(tb);

  /************************************************************/

  printf("[WRITEBACK]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
