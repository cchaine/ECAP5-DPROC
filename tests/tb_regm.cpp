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

#define NUM_TESTCASES 8

uint32_t random(uint32_t max) {
  return rand() % max;
}

/*
 * Test cases :
 *   a. Two reads at differents addresses, no write
 *   b. Two reads at the same addresses, no write
 *   c. Two reads at different addresses, write at a different address
 *   d. Two reads at different addresses, write at one of the read address
 *   e. Two reads at the same address, write at a different address
 *   f. Two reads at the same address, write at the same address
 *   g. Write to x0
 *   h. Read x0
 */
uint8_t   tc_raddr1[8] = {               0x1,                0x5,                0x7,                0x2,               0x1D,               0x1E,         random(32),                  0};
uint8_t   tc_raddr2[8] = {               0xC,                0x5,               0x1B,               0x1F,               0x1D,               0x1E,         random(32),         random(32)};
uint8_t   tc_waddr[8]  = {        random(32),         random(32),               0x10,                0x2,                0x9,               0x1E,                  0,         random(32)};
uint32_t  tc_wdata[8]  = {random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF), random(0xFFFFFFFF)};
bool      tc_write[8]  = {                 0,                  0,                  1,                  1,                  1,                  1,                  1,                  0};

struct reg_in_t {
  uint8_t   raddr1;
  uint8_t   raddr2;
  uint8_t   waddr;
  uint32_t  wdata;
  bool      write;
}; 

struct reg_out_t {
  uint32_t rdata1;
  uint32_t rdata2; 
}; 

struct reg_in_t generate_tx() {
  static int i = 0;

  struct reg_in_t tx;
  tx.raddr1 = tc_raddr1[i];
  tx.raddr2 = tc_raddr2[i];
  tx.waddr  = tc_waddr[i];
  tx.wdata  = tc_wdata[i];
  tx.write  = tc_write[i];

  i += 1;


  return tx;
}

void drive(Vregm * dut, struct reg_in_t * tx) {
  dut->raddr1_i = tx->raddr1; 
  dut->raddr2_i = tx->raddr2; 
  dut->waddr_i = tx->waddr; 
  dut->wdata_i = tx->wdata; 
  dut->write_i = tx->write;

  dut->eval();
}

uint32_t registers[32] = {0};
void monitor(Vregm * dut, int sim_time) {
  struct reg_in_t tx_in;
  tx_in.raddr1 = dut->raddr1_i; 
  tx_in.raddr2 = dut->raddr2_i; 
  tx_in.waddr = dut->waddr_i; 
  tx_in.wdata = dut->wdata_i; 
  tx_in.write = dut->write_i;

  struct reg_out_t tx_out;
  tx_out.rdata1 = dut->rdata1_o;
  tx_out.rdata2 = dut->rdata2_o;

  if(tx_out.rdata1 != registers[tx_in.raddr1]) {
    printf("\nREGM: read mismatch on port 1\n");
    printf("  Expected: 0x%08X\n", registers[tx_in.raddr1]);
    printf("  Actual:   0x%08X\n", tx_out.rdata1);
    printf("  Simtime:  %d\n", sim_time);
  }
  if(tx_out.rdata2 != registers[tx_in.raddr2]) {
    printf("\nREGM: read mismatch on port 2\n");
    printf("  Expected: 0x%08X\n", registers[tx_in.raddr2]);
    printf("  Actual:   0x%08X\n", tx_out.rdata2);
    printf("  Simtime:  %d\n", sim_time);
  }

  if(tx_in.write && (tx_in.waddr != 0)) {
    registers[tx_in.waddr] = tx_in.wdata;
  }
}

#define START_SIM_TIME 0
#define MAX_SIM_TIME (START_SIM_TIME + 2*NUM_TESTCASES)
int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));

  Vregm *dut = new Vregm;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  dut->trace(m_trace, 5);
  m_trace->open("waves/regm.vcd");

  int sim_time = 0;
  struct reg_in_t tx;
  while(sim_time < MAX_SIM_TIME) {
    dut->clk_i ^= 1;
    dut->eval();

    if(dut->clk_i == 1) {
      if(START_SIM_TIME <= sim_time) {
        tx = generate_tx();

        drive(dut, &tx);

        monitor(dut, sim_time);
      }
    }

    m_trace->dump(sim_time);
    sim_time++;
  }

  m_trace->close();
  dut->final();
  delete dut;
  exit(EXIT_SUCCESS);
}
