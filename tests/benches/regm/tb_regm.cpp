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
  delete core;
  tb->core = NULL;
}

void testbench_tick(struct testbench_t * tb) {
  Vregm * core = testbench_get_core(tb);
  tb->tickcount += 1;

  core->clk_i = 0;
  core->eval();

  core->clk_i = 1;
  core->eval();

  core->clk_i = 0;
  core->eval();
}

bool testbench_done(struct testbench_t * tb) {
  return (tb->tickcount > 10);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));

  //Verilated::traceEverOn(true);
  //VerilatedVcdC *m_trace = new VerilatedVcdC;
  //dut->trace(m_trace, 5);
  //m_trace->open("waves/regm.vcd");

  struct testbench_t tb = testbench_init();

  while(!testbench_done(&tb)) {
    testbench_tick(&tb);
  }

  //m_trace->close();
  testbench_delete(&tb);
  exit(EXIT_SUCCESS);
}
