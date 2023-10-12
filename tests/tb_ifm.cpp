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

#include "Vifm.h"

#define NUM_TESTCASES 3

uint32_t random(uint32_t max) {
  return rand() % max;
}

///*
// * Test cases :
// *   a. Two reads at differents addresses, no write
// *   b. Two reads at the same addresses, no write
// *   c. Two reads at different addresses, write at a different address
// *   d. Two reads at different addresses, write at one of the read address
// *   e. Two reads at the same address, write at a different address
// *   f. Two reads at the same address, write at the same address
// *   g. Write to x0
// */

bool tc_interrupt[4]         = {0, 0,  0};
bool tc_debug[4]             = {0, 0,  0};
bool tc_branch[4]            = {0, 0,  0};
uint32_t tc_branch_offset[4] = {0, 0, 18};

struct ifm_in_t {
  bool      interrupt;
  bool      debug;
  bool      branch;
  uint32_t  branch_offset;

}; 

struct ifm_out_t {

}; 

struct ifm_in_t generate_tx() {
  static int i = 0;

  struct ifm_in_t tx;
  tx.interrupt      =  tc_interrupt[i];
  tx.debug          =  tc_debug[i];
  tx.branch         =  tc_branch[i];
  tx.branch_offset  =  tc_branch_offset[i];

  i += 1;

  return tx;
}

void drive(Vifm * dut, struct ifm_in_t * tx) {
  dut->irq_i      =  tx->interrupt;
  dut->drq_i      =  tx->debug;
  dut->branch_i   =  tx->branch;
  dut->boffset_i  =  tx->branch_offset;

  if(dut->wb_cyc_o && dut->wb_stb_o) {
    dut->wb_dat_i = 0xCAFECAFE;
    dut->wb_ack_i = 1;
  } else {
    dut->wb_ack_i = 0;
  }

  dut->eval();
}

void monitor(Vifm * dut, int sim_time) {
}

#define START_SIM_TIME 4
#define MAX_SIM_TIME (START_SIM_TIME + 2*NUM_TESTCASES + 3)
int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));

  Vifm *dut = new Vifm;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  dut->trace(m_trace, 5);
  m_trace->open("waves/ifm.vcd");

  int sim_time = 0;
  struct ifm_in_t tx;
  while(sim_time < MAX_SIM_TIME) {
    dut->clk_i ^= 1;
    dut->rst_i = (sim_time < START_SIM_TIME);

    if(dut->clk_i == 1) {
      if(START_SIM_TIME <= sim_time) {
        tx = generate_tx();

        drive(dut, &tx);

        monitor(dut, sim_time);
      }
    } else {
      dut->eval();
    }

    m_trace->dump(sim_time);
    sim_time++;

  }

  m_trace->close();
  dut->final();
  delete dut;
  exit(EXIT_SUCCESS);
}
