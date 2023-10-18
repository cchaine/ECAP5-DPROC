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

#include "Vexm.h"

#define NUM_TESTCASES 36

uint32_t random(uint32_t max) {
  return rand() % max;
}

typedef enum { 
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BLTU, BGE, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADD, ADDI, SUB, XOR, XORI, OR, ORI, AND, ANDI, SLT, SLTI, SLTU, SLTIU, SLL, SLLI, SRL, SRLI, SRA, SRAI
} instr_t;


uint32_t  tc_pc[]     =  {0x0,  0x0,    0x0,  0x0,   0x0,  0x0,  0x0,  0x0,   0x0,  0x0,   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,   0x0,  0x0,  0x0,   0x0,  0x0,  0x0,  0x0,   0x0,  0x0,   0x0,   0x0,    0x0,  0x0,   0x0,  0x0,   0x0,  0x0};
instr_t   tc_instr[]  =  {LUI,  AUIPC,  JAL,  JALR,  BEQ,  BNE,  BLT,  BLTU,  BGE,  BGEU,  LB,   LH,   LW,   LBU,  LHU,  SB,   SH,   SW,   ADD,  ADDI,  SUB,  XOR,  XORI,  OR,   ORI,  AND,  ANDI,  SLT,  SLTI,  SLTU,  SLTIU,  SLL,  SLLI,  SRL,  SRLI,  SRA,  SRAI};
uint32_t  tc_param1[]     =  {0x1,  0x2,    0x3,  0x4,   0x4,  0x5,  0x6,  0x7,   0x8,  0x9,   0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,   0x20,  0x21,  0x22,   0x23,  0x24,  0x25,  0x26,   0x27,  0x28,   0x29,   0x30,    0x31,  0x32,   0x33,  0x34,   0x35,  0x36};
uint32_t  tc_param2[]     =  {0x1,  0x2,    0x3,  0x4,   0x4,  0x5,  0x6,  0x7,   0x8,  0x9,   0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,   0x20,  0x21,  0x22,   0x23,  0x24,  0x25,  0x26,   0x27,  0x28,   0x29,   0x30,    0x31,  0x32,   0x33,  0x34,   0x35,  0x36};
uint32_t  tc_param3[]     =  {0x1,  0x2,    0x3,  0x4,   0x4,  0x5,  0x6,  0x7,   0x8,  0x9,   0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,   0x20,  0x21,  0x22,   0x23,  0x24,  0x25,  0x26,   0x27,  0x28,   0x29,   0x30,    0x31,  0x32,   0x33,  0x34,   0x35,  0x36};

struct exm_in_t {
  uint32_t pc;
  instr_t instr;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
}; 

struct exm_out_t {

}; 

struct exm_in_t generate_tx() {
  static int i = 0;

  struct exm_in_t tx;
  tx.pc = tc_pc[i];
  tx.instr = tc_instr[i];
  tx.param1 = tc_param1[i];
  tx.param2 = tc_param2[i];
  tx.param3 = tc_param3[i];

  i += 1;

  return tx;
}

void drive(Vexm * dut, struct exm_in_t * tx) {
  dut->input_valid_i = 1;
  dut->pc_i = tx->pc;
  dut->instr_i = tx->instr;
  dut->param1_i = tx->param1;
  dut->param2_i = tx->param2;
  dut->param3_i = tx->param3;

  dut->eval_step();
}

void monitor(Vexm * dut, int sim_time) {
}

#define START_SIM_TIME 0
#define MAX_SIM_TIME (START_SIM_TIME + 2*NUM_TESTCASES + 3)
int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));

  Vexm *dut = new Vexm;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  dut->trace(m_trace, 5);
  m_trace->open("waves/exm.vcd");

  int sim_time = 0;
  struct exm_in_t tx;
  while(sim_time < MAX_SIM_TIME) {
    dut->clk_i ^= 1;

    if(dut->clk_i == 1) {
      if(START_SIM_TIME <= sim_time) {
        tx = generate_tx();

        drive(dut, &tx);

        monitor(dut, sim_time);
      }
    }

    dut->eval_step();

    dut->eval_end_step();
    m_trace->dump(sim_time);
    sim_time++;
  }

  m_trace->close();
  dut->final();
  delete dut;
  exit(EXIT_SUCCESS);
}
