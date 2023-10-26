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

#include "testsuite.h"

#include "Vexm.h"

#define RESET_DURATION 5
#define NUM_INSTRUCTIONS 37

enum instr_t { 
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BLTU, BGE, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADD, ADDI, SUB, XOR, XORI, OR, ORI, AND, ANDI, SLT, SLTI, SLTU, SLTIU, SLL, SLLI, SRL, SRLI, SRA, SRAI
};

enum instr_t str_to_instr(char * str) {
  const char * instructions[] = {
    "LUI", "AUIPC", "JAL", "JALR", "BEQ", "BNE", "BLT", "BLTU", "BGE", "BGEU", "LB", "LH", "LW", "LBU", "LHU", "SB", "SH", "SW", "ADD", "ADDI", "SUB", "XOR", "XORI", "OR", "ORI", "AND", "ANDI", "SLT", "SLTI", "SLTU", "SLTIU", "SLL", "SLLI", "SRL", "SRLI", "SRA", "SRAI"
  };
  int result = 0;
  int index = 0;
  bool found = false;
  while(!found && (index < NUM_INSTRUCTIONS)) {
    found = (strncmp(instructions[index], str, 5) == 0);
    index += (found ? 0 : 1);
  }
  if(!found) {
    printf("[ERROR]: Unknown instruction %s\n", str);
  }
  return (enum instr_t)index; 
}

struct input_vector_t {
  bool reset;
  uint32_t pc;
  instr_t instr;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
}; 

struct output_vector_t {

}; 

void initialize_testdata(struct testsuite_t testsuite) {

}

struct input_vector_t get_input_vector(struct testcase_t * testcase, bool * last_vector) {
  static int reset_count = 0;
  static int iteration_index = 0;
  *last_vector = false;

  int err;
  struct input_vector_t iv;
  if(reset_count < RESET_DURATION) {
    iv.reset = 1;
    iv.pc = 0;
    iv.param1 = 0;
    iv.param2 = 0;
    iv.param3 = 0;
    iv.instr = (enum instr_t)0;
  } else {
    iv.reset = 0;
    iv.pc = testcase_get_unsigned_int_value(testcase, "pc", iteration_index, &err);
    iv.param1 = testcase_get_unsigned_int_value(testcase, "param1", iteration_index, &err);
    iv.param2 = testcase_get_unsigned_int_value(testcase, "param2", iteration_index, &err);
    iv.param3 = testcase_get_unsigned_int_value(testcase, "param3", iteration_index, &err);
    char * instr_str = testcase_get_string_value(testcase, "instr", iteration_index, &err);
    iv.instr = str_to_instr(instr_str);
  }
   
  if(reset_count < RESET_DURATION) {
    reset_count += 1;
  } else {
    iteration_index += 1;
  }
  if(iteration_index == testcase->parameters[0].num_values) {
    reset_count = 0;
    iteration_index = 0;
    *last_vector = true;
  }

  return iv;
}

void drive_input_vector(Vexm * dut, struct input_vector_t iv) {
  dut->rst_i = iv.reset;
  dut->input_valid_i = 1;
  dut->pc_i = iv.pc;
  dut->instr_i = iv.instr;
  dut->param1_i = iv.param1;
  dut->param2_i = iv.param2;
  dut->param3_i = iv.param3;
}

void monitor_output_vector() {

}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));

  struct testsuite_t testsuite = testsuite_init(argv[1]);

  Vexm *dut = new Vexm;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  dut->trace(m_trace, 5);
  m_trace->open("waves/exm.vcd");

  int sim_time = 0;
  bool testsuite_done = false;
  int testcase_index = 0;
  bool last_vector;
  while(!testsuite_done) {
    dut->clk_i ^= 1;

    if(dut->clk_i == 1) {
      struct input_vector_t iv = get_input_vector(&testsuite.testcases[testcase_index], &last_vector);
      drive_input_vector(dut, iv);
//    monitor_output_vector();
//
      if(last_vector) {
        testcase_index += 1;
        if(testcase_index == testsuite.num_testcases) {
          testsuite_done = true;
        }
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
  testsuite_delete(testsuite);
  exit(EXIT_SUCCESS);
}
