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

#include "Vtb_ifm.h"
#include "testbench.h"
#include "Vtb_ifm_tb_ifm.h"
#include "Vtb_ifm_ecap5_dproc_pkg.h"

class TB_Ifm : public Testbench<Vtb_ifm> {
public:
  void reset() {
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;
  }
};

void tb_ifm_no_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  tb->reset();
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  tb->tick();
  // check the boot address
  CHECK("tb_ifm_no_stall_01",
      core->tb_ifm->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address,
      "Failed to initialize pc after reset");

  // check the memory read request
  CHECK("tb_ifm_no_stall_02",
    (core->tb_ifm->wb_stb_o == 1) && (core->tb_ifm->wb_cyc_o == 1),
    "Failed to perform the memory request");

  // set the injected data
  uint32_t data = rand();
  core->injected_data_i = data;
  // the memory shall have acknowledged the request 
  tb->tick();

  // check that the request is done and the value is outputed
  CHECK("tb_ifm_no_stall_03",
    (core->tb_ifm->wb_stb_o == 0) && (core->tb_ifm->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm_no_stall_04",
    core->output_valid_o == 1,
    "Failed to output the instruction");
  
  // check the stage output instruction
  CHECK("tb_ifm_no_stall_05",
    core->instr_o == data,
    "Wrong outputed instruction");

  tb->tick();

  // check the incremented address
  CHECK("tb_ifm_no_stall_06",
    core->tb_ifm->wb_adr_o == (Vtb_ifm_ecap5_dproc_pkg::boot_address + 4),
    "Failed to increment pc");
  
  // check the memory read request
  CHECK("tb_ifm_no_stall_07",
    (core->tb_ifm->wb_stb_o == 1) && (core->tb_ifm->wb_cyc_o == 1),
    "Failed to perform the subsequent memory request");
}

void tb_ifm_memory_stall(TB_Ifm * tb) {
  tb->reset();
}

void tb_ifm_memory_wait_state(TB_Ifm * tb) {
  tb->reset();
}

void tb_ifm_pipeline_stall(TB_Ifm * tb) {
  tb->reset();
}

void tb_ifm_branch(TB_Ifm * tb) {
  tb->reset();
}

void tb_ifm_interrupt(TB_Ifm * tb) {
  tb->reset();
}

void tb_ifm_debug(TB_Ifm * tb) {
  tb->reset();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  TB_Ifm * tb = new TB_Ifm;
  tb->open_trace("waves/ifm.vcd");
  tb->open_testdata("testdata/ifm.csv");

  /************************************************************/
  
  tb_ifm_no_stall(tb);

  tb_ifm_memory_stall(tb);

  tb_ifm_memory_wait_state(tb);
  
  tb_ifm_pipeline_stall(tb);

  tb_ifm_branch(tb);

  tb_ifm_interrupt(tb);

  tb_ifm_debug(tb);

  /************************************************************/

  int return_code;
  printf("[IFM]: ");
  if(tb->success) {
    printf("Done\n");
    return_code = EXIT_SUCCESS;
  } else {
    printf("Failed\n");
    return_code = EXIT_FAILURE;
  }

  delete tb;
  exit(return_code);
}
