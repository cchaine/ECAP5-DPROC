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

#include "Vregm.h"
#include "testbench.h"

struct testbench_t testbench_init() {
  struct testbench_t tb;
  tb.tickcount = 0;
  tb.core = new Vregm; 
  return tb;
}

Vregm * testbench_get_core(struct testbench_t * tb) {
  return (Vregm*)tb->core;
}

void testbench_delete(struct testbench_t * tb) {
  Vregm * core = testbench_get_core(tb);
  core->final();
  tb->trace->close();
  tb->trace = NULL;
  delete core;
  tb->core = NULL;
}

void testbench_open_trace(struct testbench_t * tb, const char *path) {
  if(tb->trace == NULL) {
    tb->trace = new VerilatedVcdC;
    Vregm * core = testbench_get_core(tb);
    core->trace(tb->trace, 99);
    tb->trace->open(path);
  }
}

void testbench_tick(struct testbench_t * tb) {
  Vregm * core = testbench_get_core(tb);
  tb->tickcount += 1;

  core->clk_i = 0;
  core->eval();
  if(tb->trace) {
    tb->trace->dump(10 * tb->tickcount - 2);
  }

  core->clk_i = 1;
  core->eval();
  if(tb->trace) {
    tb->trace->dump(10 * tb->tickcount);
  }

  core->clk_i = 0;
  core->eval();
  if(tb->trace) {
    tb->trace->dump(10 * tb->tickcount+5);
    tb->trace->flush();
  }
}

bool testbench_done(struct testbench_t * tb) {
  return (tb->tickcount > 10);
}

void testbench_set_register(struct testbench_t * tb, uint8_t addr, uint32_t value) {
  const svScope scope = svGetScopeFromName("TOP.regm");
  assert(scope);
  svSetScope(scope);
  Vregm * core = testbench_get_core(tb);
  core->set_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)&value); 
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  struct testbench_t tb = testbench_init();
  testbench_open_trace(&tb, "waves/regm.vcd");

  Vregm * core = testbench_get_core(&tb);

  /*************************************************************
   * testbench:   tb_regm_read_port_a
   * description: 
   *  Read each register through port A
   ************************************************************/
  for(int i = 0; i < 32; i++) {
    // set a random value inside the register to be read
    uint32_t value = rand();
    testbench_set_register(&tb, i, value); 

    // set core inputs
    core->raddr1_i = i;
    core->write_i = 0;
    testbench_tick(&tb);

    if(core->rdata1_o != value) {
      printf("[tb_regm_read_port_a]: Reading from register %d on port A. expected 0x%x, got 0x%x\n", i, value, core->rdata1_o);
    }
  }
  
  /*************************************************************
   * testbench:   tb_regm_read_port_b
   * description: 
   *  Read each register through port B
   ************************************************************/
  for(int i = 0; i < 32; i++) {
    // set a random value inside the register to be read
    uint32_t value = rand();
    testbench_set_register(&tb, i, value); 

    // set core inputs
    core->raddr2_i = i;
    core->write_i = 0;
    testbench_tick(&tb);

    if(core->rdata2_o != value) {
      printf("[tb_regm_read_port_b]: Reading from register %d on port B. expected 0x%x, got 0x%x\n", i, value, core->rdata2_o);
    }
  }
  
  // reset register x0
  testbench_set_register(&tb, 0, 0);

  /*************************************************************
   * testbench:   tb_regm_write_x0
   * description: 
   *  Write to the x0 register
   ************************************************************/
  core->waddr_i = 0;
  core->wdata_i = rand();
  core->write_i = 1;
  testbench_tick(&tb);
  // read back x0
  core->raddr1_i = 0;
  core->write_i = 0;
  testbench_tick(&tb);
  if(core->rdata1_o != 0) {
    printf("[tb_regm_write_x0]: While writing into register x0. Reading back, expected 0x0, got 0x%x\n", core->rdata1_o);
  }

  /*************************************************************
   * testbench:   tb_regm_write
   * description: 
   *  Write to the each register then read back from them
   ************************************************************/
  for(int i = 1; i < 32; i++) {
    uint32_t value = rand();
    core->waddr_i = i;
    core->wdata_i = value;
    core->write_i = 1;
    testbench_tick(&tb);

    core->raddr1_i = i;
    core->write_i = 0;
    testbench_tick(&tb);

    if(core->rdata1_o != value) {
      printf("[tb_regm_write]: While writing into register %i. Reading back, expected 0x%x, got 0x%x\n", i, value, core->rdata1_o);
    }
  }

  /*************************************************************
   * testbench:   tb_regm_parallel_read
   * description: 
   *  Read the same register from both ports
   ************************************************************/
  uint32_t value = rand();
  testbench_set_register(&tb, 5, value);
  core->raddr1_i = 5;
  core->raddr2_i = 5;
  core->write_i = 0;
  testbench_tick(&tb);
  if(core->rdata1_o != value) {
    printf("[tb_regm_parallel_read]: Failed reading from port A. expected 0x%x, got 0x%x\n", value, core->rdata1_o);
  }
  if(core->rdata2_o != value) {
    printf("[tb_regm_parallel_read]: Failed reading from port B. expected 0x%x, got 0x%x\n", value, core->rdata2_o);
  }

  /*************************************************************
   * testbench:   tb_regm_read_before_write
   * description: 
   *  Read and write to the same register
   ************************************************************/
  uint32_t previous_value = rand();
  value = rand();
  testbench_set_register(&tb, 5, previous_value);
  core->raddr1_i = 5;
  core->waddr_i = 5;
  core->wdata_i = value;
  core->write_i = 1;
  testbench_tick(&tb);
  if(core->rdata1_o != previous_value) {
    printf("[tb_regm_read_before_write]: Failed reading previous value. expected 0x%x, got 0x%x\n", previous_value, core->rdata1_o);
  }
  core->write_i = 0;
  testbench_tick(&tb);
  if(core->rdata1_o != value) {
    printf("[tb_regm_read_before_write]: Failed reading written value. expected 0x%x, got 0x%x\n", value, core->rdata1_o);
  }

  /************************************************************/

  printf("[DONE]\n");

  testbench_delete(&tb);
  exit(EXIT_SUCCESS);
}
