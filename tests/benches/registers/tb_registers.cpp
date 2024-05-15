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

#include "Vtb_registers.h"
#include "testbench.h"

enum CondId {
  COND_read,
  COND_write,
  __CondIdEnd
};

enum TestcaseId {
  T_READ_PORTA         =  1,
  T_READ_PORTB         =  2,
  T_WRITE_X0           =  3,
  T_WRITE              =  4,
  T_PARALLEL_READ      =  5,
  T_READ_BEFORE_WRITE  =  6,
  T_READ_X0            =  7,
  T_RESET              =  8
};

class TB_Registers : public Testbench<Vtb_registers> {
public:
  void reset() {
    this->set_register(0, 0);

    Testbench<Vtb_registers>::reset();
  }

  void set_register(uint8_t addr, uint32_t value) {
    const svScope scope = svGetScopeFromName("TOP.tb_registers.dut");
    assert(scope);
    svSetScope(scope);
    this->core->set_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)&value); 
  }
};

void tb_registers_reset(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_RESET;

  tb->reset();

  CHECK("tb_registers.reset.01",
      false,
    "TODO");
}

void tb_registers_read_x0(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_READ_X0;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to read x0 through port a
  //    tick 1. Set the inputs to read x0 through port b
  
  tb->reset();

  core->raddr1_i = 0;
  core->write_i = 0;

  //=================================
  //      Tick (0)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == 0));

  //`````````````````````````````````
  //      Set inputs
    
  core->raddr2_i = 0;
  tb->check(COND_read, (core->rdata2_o == 0));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_registers.read_x0.01",
    tb->conditions[COND_read],
    "Failed to read registers x0", tb->err_cycles[COND_read]);
}

void tb_registers_read_port_a(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_READ_PORTA;

  // The following actions are performed in this test :
  //    tick 0-31. Set the inputs to read i through port a

  tb->reset();

  for(int i = 1; i < 32; i++) {
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
  
  CHECK("tb_registers.read_port_a.01",
    tb->conditions[COND_read],
    "Failed to read registers from port A", tb->err_cycles[COND_read]);
}

void tb_registers_read_port_b(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_READ_PORTB;

  // The following actions are performed in this test :
  //    tick 0-31. Set the inputs to read i through port b

  tb->reset();

  for(int i = 1; i < 32; i++) {
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
  
  CHECK("tb_registers.read_port_b.01",
    tb->conditions[COND_read],
    "Failed to read registers from port B", tb->err_cycles[COND_read]);
}

void tb_registers_write_x0(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_WRITE_X0;

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
  
  CHECK("tb_registers.write_x0.01",
    tb->conditions[COND_write],
    "Failed to prevent writing to x0", tb->err_cycles[COND_write]);
}

void tb_registers_write(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_WRITE;

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
  
  CHECK("tb_registers.write.01",
    tb->conditions[COND_read],
    "Failed writing to registers", tb->err_cycles[COND_read]);
}

void tb_registers_parallel_read(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_PARALLEL_READ;

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
  
  CHECK("tb_registers.parallel_read.01",
    tb->conditions[COND_read],
    "Failed reading both port A and B in parallel", tb->err_cycles[COND_read]);
}

void tb_registers_read_before_write(TB_Registers * tb) {
  Vtb_registers * core = tb->core;
  core->testcase = T_READ_BEFORE_WRITE;

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

  // this change is asynchronous
  core->eval();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == previous_value));

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->write_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_read, (core->rdata1_o == value));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_registers.read_before_write.01",
    tb->conditions[COND_read],
    "Failed writing to registers", tb->err_cycles[COND_read]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Registers * tb = new TB_Registers();
  tb->open_trace("waves/registers.vcd");
  tb->open_testdata("testdata/registers.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_registers_reset(tb);

  tb_registers_read_x0(tb);
  tb_registers_read_port_a(tb);
  tb_registers_read_port_b(tb);

  tb_registers_write_x0(tb);
  tb_registers_write(tb);

  tb_registers_parallel_read(tb);

  tb_registers_read_before_write(tb);

  /************************************************************/

  printf("[REGISTERS]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
