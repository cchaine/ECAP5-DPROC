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

#include "Vtb_hazard.h"
#include "testbench.h"
#include "Vtb_hazard_ecap5_dproc_pkg.h"

enum CondId {
  COND_control,
  COND_data,
  __CondIdEnd
};

enum TestcaseId {
  T_CONTROL = 1,
  T_DATA_X0 = 2,
  T_DATA_PORT1 = 3,
  T_DATA_PORT2 = 4,
  T_DATA_MULTIPLE = 5,
  T_RESET = 6
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
    core->dec_reg_write_i = 0;
    core->dec_reg_addr_i = 0;
    core->ex_reg_write_i = 0;
    core->ex_reg_addr_i = 0;
    core->ls_reg_write_i = 0;
    core->ls_reg_addr_i = 0;
    core->reg_write_i = 0;
    core->reg_waddr_i = 0;
  }

};

void tb_hazard_reset(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_RESET;

  tb->reset();
  
  CHECK("tb_hazard.reset.01",
      false,
      "TODO");
}

void tb_hazard_control(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_CONTROL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for control hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 1;

  // the output is asynchrounous
  core->eval();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_control, (core->ex_discard_request_o == 1));

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_control, (core->ex_discard_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_control, (core->ex_discard_request_o == 0));

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_hazard.control.01",
      tb->conditions[COND_control],
      "Failed to protect against control hazards", tb->err_cycles[COND_control]);
}

void tb_hazard_data_x0(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_DATA_X0;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for data hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->reg_raddr1_i = 0;
  core->reg_raddr2_i = 0;

  core->dec_reg_write_i = 1;
  core->dec_reg_addr_i = 0;

  core->ex_reg_write_i = 1;
  core->ex_reg_addr_i = 0;

  core->ls_reg_write_i = 1;
  core->ls_reg_addr_i = 0;

  core->reg_write_i = 1;
  core->reg_waddr_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 0));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_hazard.data.X0_01",
      tb->conditions[COND_data],
      "Failed to protect against data hazards", tb->err_cycles[COND_data]);
}

void tb_hazard_data_port1(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_DATA_PORT1;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for data hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t reg1 = 1 + rand() % 31;
  core->reg_raddr1_i = reg1;
  core->reg_raddr2_i = 0;

  core->dec_reg_write_i = 0;
  core->dec_reg_addr_i = reg1;

  core->ex_reg_write_i = 0;
  core->ex_reg_addr_i = reg1;

  core->ls_reg_write_i = 0;
  core->ls_reg_addr_i = reg1;

  core->reg_write_i = 0;
  core->reg_waddr_i = reg1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->dec_reg_write_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->dec_reg_write_i = 0;
  core->ex_reg_write_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ex_reg_write_i = 0;
  core->ls_reg_write_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ls_reg_write_i = 0;
  core->reg_write_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_hazard.data.PORT1_01",
      tb->conditions[COND_data],
      "Failed to protect against data hazards", tb->err_cycles[COND_data]);
}

void tb_hazard_data_port2(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_DATA_PORT2;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for data hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->reg_raddr1_i = 0;
  uint32_t reg2 = 1 + rand() % 31;
  core->reg_raddr2_i = reg2;

  core->dec_reg_write_i = 0;
  core->dec_reg_addr_i = reg2;

  core->ex_reg_write_i = 0;
  core->ex_reg_addr_i = reg2;

  core->ls_reg_write_i = 0;
  core->ls_reg_addr_i = reg2;

  core->reg_write_i = 0;
  core->reg_waddr_i = reg2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->dec_reg_write_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->dec_reg_write_i = 0;
  core->ex_reg_write_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ex_reg_write_i = 0;
  core->ls_reg_write_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ls_reg_write_i = 0;
  core->reg_write_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_hazard.data.PORT2_01",
      tb->conditions[COND_data],
      "Failed to protect against data hazards", tb->err_cycles[COND_data]);
}

void tb_hazard_data_multiple(TB_Hazard * tb) {
  Vtb_hazard * core = tb->core;
  core->testcase = T_DATA_MULTIPLE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for data hazard

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t reg1 = 1 + rand() % 31;
  core->reg_raddr1_i = reg1;
  uint32_t reg2 = 1 + rand() % 31;
  core->reg_raddr2_i = reg2;

  core->dec_reg_write_i = 1;
  core->dec_reg_addr_i = reg2;

  core->ex_reg_write_i = 1;
  core->ex_reg_addr_i = reg1;

  core->ls_reg_write_i = 1;
  core->ls_reg_addr_i = reg2;

  core->reg_write_i = 1;
  core->reg_waddr_i = reg2;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->dec_reg_write_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ex_reg_write_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->ls_reg_write_i = 0;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 1));

  //`````````````````````````````````
  //      Set inputs
  
  core->reg_write_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_data, (core->dec_stall_request_o == 0));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_hazard.data.MULTIPLE_01",
      tb->conditions[COND_data],
      "Failed to protect against data hazards", tb->err_cycles[COND_data]);
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

  tb_hazard_reset(tb);

  tb_hazard_control(tb);

  tb_hazard_data_x0(tb);
  tb_hazard_data_port1(tb);
  tb_hazard_data_port2(tb);
  tb_hazard_data_multiple(tb);

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
