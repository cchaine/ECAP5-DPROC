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

#include "Vtb_regm.h"
#include "testbench.h"

class TB_Regm : public Testbench<Vtb_regm> {
public:
  void reset() {
    this->set_register(0, 0);

    Testbench<Vtb_regm>::reset();
  }

  void set_register(uint8_t addr, uint32_t value) {
    const svScope scope = svGetScopeFromName("TOP.tb_regm.dut");
    assert(scope);
    svSetScope(scope);
    this->core->set_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)&value); 
  }
};

enum CondId {
  COND_read,
  COND_write,
  __CondIdEnd
};

void tb_regm_read_port_a(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0-31. Set the inputs to read i through port a

  tb->reset();

  for(int i = 0; i < 32; i++) {
    //`````````````````````````````````
    //      Set inputs
    
    uint32_t value = rand();
    tb->set_register(i, value); 

    core->raddr1_i = i;
    core->write_i = 0;

    //=================================
    //      Tick (0-31)
    
    tb->tick();

    //`````````````````````````````````
    //      Checks 
    
    tb->check(COND_read, (core->rdata1_o == value));
  }

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.read_port_a.01",
    tb->conditions[COND_read],
    "Failed to read registers from port A", tb->err_cycles[COND_read]);
}

void tb_regm_read_port_b(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 2;

  // The following actions are performed in this test :
  //    tick 0-31. Set the inputs to read i through port b

  tb->reset();

  for(int i = 0; i < 32; i++) {
    //`````````````````````````````````
    //      Set inputs
    
    uint32_t value = rand();
    tb->set_register(i, value); 

    core->raddr2_i = i;
    core->write_i = 0;

    //=================================
    //      Tick (0-31)
    
    tb->tick();

    //`````````````````````````````````
    //      Checks 
    
    tb->check(COND_read, (core->rdata2_o == value));
  }

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.read_port_b.01",
    tb->conditions[COND_read],
    "Failed to read registers from port B", tb->err_cycles[COND_read]);
}

void tb_regm_write_x0(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 3;

  // The following actions are performed in this test :
  //    tick 0. Set inputs to write random data to x0
  //    tick 1. Set inputs to read x0

  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->waddr_i = 0;
  core->wdata_i = rand();
  core->write_i = 1;

  //=================================
  //      Tick (0)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->raddr1_i = 0;
  core->write_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_write, (core->rdata1_o == 0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.write_x0.01",
    tb->conditions[COND_write],
    "Failed to prevent writing to x0", tb->err_cycles[COND_write]);
}

void tb_regm_write(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 4;

  // The following actions are performed in this test :
  //    tick 0, 2, 4, ..., 30. Set inputs to write random data to xi
  //    tick 1, 3, 5, ..., 31. Set inputs to read xi

  tb->reset();

  for(int i = 1; i < 32; i++) {
    //`````````````````````````````````
    //      Set inputs
    
    uint32_t value = rand();
    core->waddr_i = i;
    core->wdata_i = value;
    core->write_i = 1;

    //=================================
    //      Tick (0, 2, 4, ..., 30)
    
    tb->tick();

    //`````````````````````````````````
    //      Set inputs
    
    core->raddr1_i = i;
    core->write_i = 0;

    //=================================
    //      Tick (1, 3, 5, ..., 31)
    
    tb->tick();

    //`````````````````````````````````
    //      Checks 
    
    tb->check(COND_read, (core->rdata1_o == value));
  }

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.write.01",
    tb->conditions[COND_read],
    "Failed writing to registers", tb->err_cycles[COND_read]);
}

void tb_regm_parallel_read(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 5;

  // The following actions are performed in this test :
  //    tick 0. Set inputs to read register 5 with both port a and b

  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t value = rand();
  tb->set_register(5, value);

  core->raddr1_i = 5;
  core->raddr2_i = 5;
  core->write_i = 0;

  //=================================
  //      Tick (0)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == value) &&
                       (core->rdata2_o == value));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.parallel_read.01",
    tb->conditions[COND_read],
    "Failed reading both port A and B in parallel", tb->err_cycles[COND_read]);
}

void tb_regm_read_before_write(TB_Regm * tb) {
  Vtb_regm * core = tb->core;
  core->testcase = 6;

  // The following actions are performed in this test :
  //    tick 0. Set inputs to read register 5 and write random data to register 5
  //    tick 1. Set inputs to read register 5 

  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t previous_value = rand();
  tb->set_register(5, previous_value);

  uint32_t value = rand();
  core->raddr1_i = 5;
  core->waddr_i = 5;
  core->wdata_i = value;
  core->write_i = 1;

  //=================================
  //      Tick (0)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == previous_value));

  //`````````````````````````````````
  //      Set inputs
  
  core->write_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == value));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_regm.read_before_write.01",
    tb->conditions[COND_read],
    "Failed writing to registers", tb->err_cycles[COND_read]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Regm * tb = new TB_Regm();
  tb->open_trace("waves/regm.vcd");
  tb->open_testdata("testdata/regm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_regm_read_port_a(tb);
  
  tb_regm_read_port_b(tb);

  tb_regm_write_x0(tb);

  tb_regm_write(tb);

  tb_regm_parallel_read(tb);

  tb_regm_read_before_write(tb);

  /************************************************************/

  printf("[REGM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
