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

#include "Vtb_exm.h"
#include "testbench.h"
#include "Vtb_exm_ecap5_dproc_pkg.h"

class TB_Exm : public Testbench<Vtb_exm> {
public:
  void reset() {
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;
  }
};

void tb_exm_alu_add(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
  
  core->input_valid_i = 1;
  core->output_ready_i = 1;
  core->alu_shift_right_i = 0;
  core->alu_signed_shift_i = 0;
  core->branch_offset_i = 0;
  core->result_addr_i = 0;

  // ADD
  
  uint32_t operand1 = rand();
  core->alu_operand1_i = operand1;
  uint32_t operand2 = rand();
  core->alu_operand2_i = operand2;
  core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
  core->alu_sub_i = 0;
  core->branch_cond_i = 0;
  core->result_write_i = 1;
  uint8_t result_addr1 = rand() % 32;
  core->result_addr_i = result_addr1;

  tb->tick();

  // SUB 
  
  uint32_t operand3 = rand();
  core->alu_operand1_i = operand3;
  uint32_t operand4 = rand();
  core->alu_operand2_i = operand4;
  core->alu_op_i = Vtb_exm_ecap5_dproc_pkg::ALU_ADD;
  core->alu_sub_i = 1;
  core->branch_cond_i = 0;
  core->result_write_i = 1;
  uint32_t result_addr2 = rand() % 32;
  core->result_addr_i = result_addr2;

  tb->tick();

  CHECK("tb_exm.alu.ADD_01",
      (core->result_o == ((int32_t)operand1 + (int32_t)operand2)),
      "Failed to execute ALU_ADD operation");
  CHECK("tb_exm.alu.ADD_02",
      (core->result_write_o == 1),
      "Failed to output the result write");
  CHECK("tb_exm.alu.ADD_03",
      (core->result_addr_o == result_addr1),
      "Failed to output the result address");
  CHECK("tb_exm.alu.ADD_04",
      (core->branch_o == 0),
      "Failed to output branch");
  CHECK("tb_exm.alu.ADD_05",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();

  CHECK("tb_exm.alu.ADD_06",
      (core->result_o == ((int32_t)operand3 - (int32_t)operand4)),
      "Failed to execute ALU_ADD operation with sub");
  CHECK("tb_exm.alu.ADD_07",
      (core->result_write_o == 1),
      "Failed to output the result write");
  CHECK("tb_exm.alu.ADD_08",
      (core->result_addr_o == result_addr2),
      "Failed to output the result address");
  CHECK("tb_exm.alu.ADD_09",
      (core->branch_o == 0),
      "Failed to output branch");
  CHECK("tb_exm.alu.ADD_10",
      (core->output_valid_o == 1),
      "Failed to validate the output");
}

void tb_exm_alu_xor(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_alu_or(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_alu_and(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_alu_slt(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_alu_sltu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_alu_shift(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_beq(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_bne(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_blt(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_bltu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_bge(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_branch_bgeu(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_bubble(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

void tb_exm_stall(TB_Exm * tb) {
  Vtb_exm * core = tb->core;
  tb->reset();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Exm * tb = new TB_Exm;
  tb->open_trace("waves/exm.vcd");
  tb->open_testdata("testdata/exm.csv");
  tb->set_debug_log(verbose);

  /************************************************************/
  
  tb_exm_alu_add(tb);
  tb_exm_alu_xor(tb);
  tb_exm_alu_or(tb);
  tb_exm_alu_and(tb);
  tb_exm_alu_slt(tb);
  tb_exm_alu_sltu(tb);
  tb_exm_alu_shift(tb);

  tb_exm_branch_beq(tb);
  tb_exm_branch_bne(tb);
  tb_exm_branch_blt(tb);
  tb_exm_branch_bltu(tb);
  tb_exm_branch_bge(tb);
  tb_exm_branch_bgeu(tb);

  tb_exm_bubble(tb);
  tb_exm_stall(tb);

  /************************************************************/

  printf("[EXM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
