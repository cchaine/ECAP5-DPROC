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

#include "Vtb_hazard.h"
#include "testbench.h"
#include "Vtb_hazard_ecap5_dproc_pkg.h"

enum CondId {
  COND_control,
  __CondIdEnd
};

enum TestcaseId {
  T_CONTROL
};

class TB_Hazard : public Testbench<Vtb_hazard> {
public:
  void reset() {
    this->_nop();

    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_hazard>::reset();
  }
  
  void _nop() {
  }

};

void tb_hazard_control(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_CONTROL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for control hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Hazard * tb = new TB_Hazard;
  tb->open_trace("waves/hazard.vcd");
  tb->open_testdata("testdata/hazard.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_hazard_control(tb);

  /************************************************************/

  printf("[HAZARD]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
