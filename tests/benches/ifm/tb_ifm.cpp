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

  CHECK("tb_ifm.no_stall_01",
      (core->output_valid_o == 0),
      "Failed to reset the module");

  CHECK("tb_ifm.no_stall_13",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to initialize the memory interface");

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;
  tb->tick();

  // check that the output is properly invalidated
  CHECK("tb_ifm.no_stall_15",
      (core->output_valid_o == 0),
      "Failed to properly unvalidate the output on a memory read request");

  // check the boot address
  CHECK("tb_ifm.no_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to initialize pc after reset");

  // check the memory read request
  CHECK("tb_ifm.no_stall_03",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the memory read request");
  
  // check the size of the memory read request
  CHECK("tb_ifm.no_stall_04",
    (core->wb_sel_o == 0x7),
    "Failed to specify the size of the memory read request");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.no_stall_05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.no_stall_06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.no_stall_07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.no_stall_08",
    (core->output_valid_o == 1),
    "Failed to output the instruction");
  
  // check the stage output instruction
  CHECK("tb_ifm.no_stall_09",
    (core->instr_o == data),
    "Wrong outputed instruction");
  // check the stage output pc
  CHECK("tb_ifm.no_stall_14",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Wrong outputed pc");

  tb->tick();

  // check that a successfull output handshake is properly detected
  CHECK("tb_ifm.no_stall_10",
      (core->output_valid_o == 0),
      "Failed to invalidate the output after a successfull handshake");

  // check the incremented address
  CHECK("tb_ifm.no_stall_11",
    (core->wb_adr_o == (Vtb_ifm_ecap5_dproc_pkg::boot_address + 4)),
    "Failed to increment pc");
  
  // check the memory read request
  CHECK("tb_ifm.no_stall_12",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the subsequent memory request");
}

void tb_ifm_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  // Stall the memory interface
  core->wb_stall_i = 1;
  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.stall_01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform the memory read request : 1");

  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.stall_02",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 2");

  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.stall_03",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 3");

  core->wb_stall_i = 0;
  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.stall_04",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 4");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.stall_05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.stall_06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.stall_07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");
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

  bool verbose = parse_verbose(argc, argv);

  TB_Ifm * tb = new TB_Ifm;
  tb->open_trace("waves/ifm.vcd");
  tb->open_testdata("testdata/ifm.csv");
  tb->set_debug_log(verbose);

  /************************************************************/
  
  tb_ifm_no_stall(tb);

  tb_ifm_memory_stall(tb);

  tb_ifm_memory_wait_state(tb);
  
  tb_ifm_pipeline_stall(tb);

  tb_ifm_branch(tb);

  tb_ifm_interrupt(tb);

  tb_ifm_debug(tb);

  /************************************************************/

  printf("[IFM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
