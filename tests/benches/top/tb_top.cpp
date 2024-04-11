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

#include "Vtb_top.h"
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
  __CondIdEnd
};

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

  tb->tick();

  tb->tick();

  tb->tick();

  tb->tick();

  tb->tick();

  tb->tick();

  tb->tick();

  tb->tick();
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
