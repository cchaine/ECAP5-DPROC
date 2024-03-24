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
#include "Vtb_ifm_ecap5_dproc_pkg.h"
#include "Vtb_ifm_ifm.h"
#include "Vtb_ifm_tb_ifm.h"

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
  core->testcase = 1;
  tb->reset();

  CHECK("tb_ifm.no_stall.01",
      (core->output_valid_o == 0),
      "Failed to reset the module");

  CHECK("tb_ifm.no_stall.13",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to initialize the memory interface");

  CHECK("tb_ifm.no_stall.16",
      (core->tb_ifm->dut->state_q == 0),
      "Failed to enter the IDLE state");

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;
  tb->tick();

  CHECK("tb_ifm.no_stall.17",
      (core->tb_ifm->dut->state_q == 1),
      "Failed to enter the REQUEST state");

  // check that the output is properly invalidated
  CHECK("tb_ifm.no_stall.15",
      (core->output_valid_o == 0),
      "Failed to properly unvalidate the output on a memory read request");

  // check the boot address
  CHECK("tb_ifm.no_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to initialize pc after reset");

  // check the memory read request
  CHECK("tb_ifm.no_stall.03",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the memory read request");
  
  // check the size of the memory read request
  CHECK("tb_ifm.no_stall.04",
    (core->wb_sel_o == 0xF),
    "Failed to specify the size of the memory read request");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  CHECK("tb_ifm.no_stall.18",
      (core->tb_ifm->dut->state_q == 3),
      "Failed to enter the DONE state");

  // check that the request is being processed
  CHECK("tb_ifm.no_stall.05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.no_stall.06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  CHECK("tb_ifm.no_stall.19",
      (core->tb_ifm->dut->state_q == 0),
      "Failed to enter the IDLE state");

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.no_stall.07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.no_stall.08",
    (core->output_valid_o == 1),
    "Failed to output the instruction");
  
  // check the stage output instruction
  CHECK("tb_ifm.no_stall.09",
    (core->instr_o == data),
    "Wrong outputed instruction");
  // check the stage output pc
  CHECK("tb_ifm.no_stall.14",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Wrong outputed pc");

  tb->tick();

  CHECK("tb_ifm.no_stall.20",
      (core->tb_ifm->dut->state_q == 1),
      "Failed to enter the REQUEST state");

  // check that a successfull output handshake is properly detected
  CHECK("tb_ifm.no_stall.10",
      (core->output_valid_o == 0),
      "Failed to invalidate the output after a successfull handshake");

  // check the incremented address
  CHECK("tb_ifm.no_stall.11",
    (core->wb_adr_o == (Vtb_ifm_ecap5_dproc_pkg::boot_address + 4)),
    "Failed to increment pc");
  
  // check the memory read request
  CHECK("tb_ifm.no_stall.12",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the subsequent memory request");
}

void tb_ifm_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 2;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  // Stall the memory interface
  core->wb_stall_i = 1;
  tb->tick();

  CHECK("tb_ifm.memory_stall.08",
      (core->tb_ifm->dut->state_q == 4),
      "Failed to enter the MEMORY_STALL state");

  // check the memory read request
  CHECK("tb_ifm.memory_stall.01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform the memory read request : 1st time");

  tb->tick();

  CHECK("tb_ifm.memory_stall.09",
      (core->tb_ifm->dut->state_q == 4),
      "Failed to enter the MEMORY_STALL state");

  // check the memory read request
  CHECK("tb_ifm.memory_stall.02",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 2nd time");

  tb->tick();

  CHECK("tb_ifm.memory_stall.10",
      (core->tb_ifm->dut->state_q == 4),
      "Failed to enter the MEMORY_STALL state");

  // check the memory read request
  CHECK("tb_ifm.memory_stall.03",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 3rd time");

  core->wb_stall_i = 0;
  tb->tick();

  CHECK("tb_ifm.memory_stall.11",
      (core->tb_ifm->dut->state_q == 1),
      "Failed to enter the REQUEST state");

  // check the memory read request
  CHECK("tb_ifm.memory_stall.04",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : final time");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  CHECK("tb_ifm.memory_stall.12",
      (core->tb_ifm->dut->state_q == 3),
      "Failed to enter the DONE state");

  // check that the request is being processed
  CHECK("tb_ifm.memory_stall.05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_stall.06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  CHECK("tb_ifm.memory_stall.13",
      (core->tb_ifm->dut->state_q == 0),
      "Failed to enter the IDLE state");

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.memory_stall.07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");
}

void tb_ifm_memory_wait_state(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 3;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;
  tb->tick();

  // read request is sent
  
  CHECK("tb_ifm.memory_wait_state.08",
      (core->tb_ifm->dut->state_q == 1),
      "Failed to enter the REQUEST state");
  
  // insert one wait cycle
  tb->tick();

  CHECK("tb_ifm.memory_wait_state.09",
      (core->tb_ifm->dut->state_q == 2),
      "Failed to enter the MEMORY_WAIT state");

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  // insert second wait cycle
  tb->tick();

  CHECK("tb_ifm.memory_wait_state.10",
      (core->tb_ifm->dut->state_q == 2),
      "Failed to enter the MEMORY_WAIT state");

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state.03",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state.04",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  CHECK("tb_ifm.memory_wait_state.11",
      (core->tb_ifm->dut->state_q == 3),
      "Failed to enter the DONE state");

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state.05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state.06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  CHECK("tb_ifm.memory_wait_state.12",
      (core->tb_ifm->dut->state_q == 0),
      "Failed to enter the IDLE state");

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.memory_wait_state.07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");
}

void tb_ifm_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 4;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  // Stall the pipeline
  core->output_ready_i = 0;
  core->wb_stall_i = 0;
  tb->tick();

  // read request is sent
  
  CHECK("tb_ifm.pipeline_stall.16",
      (core->tb_ifm->dut->state_q == 1),
      "Failed to enter the REQUEST state");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  CHECK("tb_ifm.pipeline_stall.17",
      (core->tb_ifm->dut->state_q == 3),
      "Failed to enter the DONE state");

  // the request is being processed

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  CHECK("tb_ifm.pipeline_stall.18",
      (core->tb_ifm->dut->state_q == 5),
      "Failed to enter the PIPELINE_STALL state");

  // the response has been received

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall.01",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall.02",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall.03",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall.04",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  tb->tick();

  CHECK("tb_ifm.pipeline_stall.19",
      (core->tb_ifm->dut->state_q == 5),
      "Failed to enter the PIPELINE_STALL state");

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall.05",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall.06",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall.07",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall.08",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  core->output_ready_i = 1;

  tb->tick();

  CHECK("tb_ifm.pipeline_stall.20",
      (core->tb_ifm->dut->state_q == 0),
      "Failed to enter the IDLE state");

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall.09",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall.10",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall.11",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall.12",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  tb->tick();

  // check that a successfull output handshake is properly detected
  CHECK("tb_ifm.pipeline_stall.13",
      (core->output_valid_o == 0),
      "Failed to invalidate the output after a successfull handshake");

  // check the incremented address
  CHECK("tb_ifm.pipeline_stall.14",
    (core->wb_adr_o == (Vtb_ifm_ecap5_dproc_pkg::boot_address + 4)),
    "Failed to increment pc");
  
  // check the memory read request
  CHECK("tb_ifm.pipeline_stall.15",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the subsequent memory request");
}

/*============================================*/
/*                   Debug                    */
/*============================================*/

void tb_ifm_debug_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 5;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;

  // check the boot address
  CHECK("tb_ifm.debug_during_request.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  
  // check the stage output
  CHECK("tb_ifm.debug_during_request.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_request.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 6;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;

  // check that the request is being processed
  CHECK("tb_ifm.debug_during_ack.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.debug_during_ack.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.debug_during_ack.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_during_ack.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_ack.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 7;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  tb->tick();

  // waiting for ack

  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;

  // waiting for ack

  tb->tick();

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.debug_during_wait.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_during_wait.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_wait.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 8;
  tb->reset();
  
  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 1;
  
  tb->tick();
  
  // request ready
  
  tb->tick();
  
  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;
  tb->tick();
  core->wb_stall_i = 0;
  tb->tick();
  
  CHECK("tb_ifm.debug_during_memory_stall.01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");
  
  CHECK("tb_ifm.debug_during_memory_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;
  
  tb->tick();
  
  // check the stage output
  CHECK("tb_ifm.debug_during_memory_stall.03",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();
  
  CHECK("tb_ifm.debug_during_memory_stall.04",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 9;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.debug_on_output_handshake.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_on_output_handshake.02",
    (core->output_valid_o == 1),
    "Failed to output the pre-jump instruction");
  
  tb->tick();

  CHECK("tb_ifm.debug_on_output_handshake.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 10;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 0;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // output ready

  core->drq_i = 1;
  tb->tick();
  core->drq_i = 0;
  tb->tick();

  // check the stage output
  CHECK("tb_ifm.debug_during_pipeline_stall.01",
    (core->output_valid_o == 0),
    "Failed to cancel output");

  tb->tick();
  
  CHECK("tb_ifm.debug_during_pipeline_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

void tb_ifm_debug_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 11;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->drq_i = 1;
  tb->tick();
  core->drq_i = 1;

  // check that the request is being processed
  CHECK("tb_ifm.debug_back_to_back.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.debug_back_to_back.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->drq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.debug_back_to_back.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_back_to_back.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_back_to_back.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to fetch the debug instruction");
}

/*============================================*/
/*                 Interrupt                  */
/*============================================*/

void tb_ifm_interrupt_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 12;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;

  // check the boot address
  CHECK("tb_ifm.interrupt_during_request.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");

  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check the stage output
  CHECK("tb_ifm.interrupt_during_request.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");

  tb->tick();

  CHECK("tb_ifm.interrupt_during_request.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interrupt instruction");
}

void tb_ifm_interrupt_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 13;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;

  // check that the request is being processed
  CHECK("tb_ifm.interrupt_during_ack.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.interrupt_during_ack.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.interrupt_during_ack.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_during_ack.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_ack.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 14;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  tb->tick();

  // waiting for ack

  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;

  // waiting for ack

  tb->tick();

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.interrupt_during_wait.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_during_wait.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_wait.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 15;
  tb->reset();
  
  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 1;
  
  tb->tick();
  
  // request ready
  
  tb->tick();
  
  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;
  tb->tick();
  core->wb_stall_i = 0;
  tb->tick();
  
  CHECK("tb_ifm.interrupt_during_memory_stall.01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");
  
  CHECK("tb_ifm.interrupt_during_memory_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;
  
  tb->tick();
  
  // check the stage output
  CHECK("tb_ifm.interrupt_during_memory_stall.03",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();
  
  CHECK("tb_ifm.interrupt_during_memory_stall.04",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interrupt instruction");
}

void tb_ifm_interrupt_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 16;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.interrupt_on_output_handshake.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_on_output_handshake.02",
    (core->output_valid_o == 1),
    "Failed to output the pre-jump instruction");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_on_output_handshake.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 17;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 0;
  core->wb_stall_i = 0;

  tb->tick();

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // output ready

  core->irq_i = 1;
  tb->tick();
  core->irq_i = 0;
  tb->tick();

  // check the stage output
  CHECK("tb_ifm.interrupt_during_pipeline_stall.01",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_pipeline_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 18;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->irq_i = 1;
  tb->tick();
  core->irq_i = 1;

  // check that the request is being processed
  CHECK("tb_ifm.interrupt_back_to_back.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.interrupt_back_to_back.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->irq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.interrupt_back_to_back.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_back_to_back.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_back_to_back.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

/*============================================*/
/*                   Branch                   */
/*============================================*/

void tb_ifm_branch_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 19;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;

  // check the boot address
  CHECK("tb_ifm.branch_during_request.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");

  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check the stage output
  CHECK("tb_ifm.branch_during_request.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");

  tb->tick();

  CHECK("tb_ifm.branch_during_request.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the branch instruction");
}

void tb_ifm_branch_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 20;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;

  // check that the request is being processed
  CHECK("tb_ifm.branch_during_ack.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.branch_during_ack.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.branch_during_ack.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_during_ack.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_ack.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 21;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  tb->tick();
  
  // waiting for ack

  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;

  // waiting for ack

  tb->tick();

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.branch_during_wait.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_during_wait.02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_wait.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 22;
  tb->reset();
  
  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 1;
  
  tb->tick();
  
  // request ready
  
  tb->tick();
  
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;
  tb->tick();
  core->wb_stall_i = 0;
  tb->tick();
  
  CHECK("tb_ifm.branch_during_memory_stall.01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");
  
  CHECK("tb_ifm.branch_during_memory_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
      "Failed to fetch the pre-jump instruction");
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;
  
  tb->tick();
  
  // check the stage output
  CHECK("tb_ifm.branch_during_memory_stall.03",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();
  
  CHECK("tb_ifm.branch_during_memory_stall.04",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the branch instruction");
}

void tb_ifm_branch_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 23;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;

  // check that the request is done
  CHECK("tb_ifm.branch_on_output_handshake.01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_on_output_handshake.02",
    (core->output_valid_o == 1),
    "Failed to output the pre-jump instruction");
  
  tb->tick();

  CHECK("tb_ifm.branch_on_output_handshake.03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 24;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 0;
  core->wb_stall_i = 0;

  tb->tick();

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // output ready

  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;
  tb->tick();

  // check the stage output
  CHECK("tb_ifm.branch_during_pipeline_stall.01",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_pipeline_stall.02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 25;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->branch_i = 1;
  uint32_t first_offset = rand() % 0x7FFFFFFF;
  core->boffset_i = first_offset;
  tb->tick();
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;

  // check that the request is being processed
  CHECK("tb_ifm.branch_back_to_back.01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.branch_back_to_back.02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->branch_i = 0;

  // check that the request is done
  CHECK("tb_ifm.branch_back_to_back.03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_back_to_back.04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_back_to_back.05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + first_offset),
      "Failed to fetch the branch instruction");
}

/*============================================*/
/*                 Precedence                 */
/*============================================*/

void tb_ifm_precedence_debug(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 26;
  tb->reset();

  // Leave the reset state
  core->drq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->drq_i = 1;
  core->irq_i = 1;
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;
  core->irq_i = 0;
  core->drq_i = 0;
  
  tb->tick();

  CHECK("tb_ifm.precedence_debug.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address),
      "Failed to update pc with the right precedence");
}

void tb_ifm_precedence_interrupt(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 27;
  tb->reset();

  // Leave the reset state
  core->drq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->irq_i = 1;
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;
  core->irq_i = 0;
  
  tb->tick();

  CHECK("tb_ifm.precedence_interrupt.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to update pc with the right precedence");
}

void tb_ifm_precedence_branch(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 28;
  tb->reset();

  // Leave the reset state
  core->drq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;
  tb->tick();
  core->branch_i = 0;
  
  tb->tick();

  CHECK("tb_ifm.precedence_branch.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to update pc with the right precedence");
}

void tb_ifm_precedence_increment(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 29;
  tb->reset();

  // Leave the reset state
  core->drq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  tb->tick();

  // request sent

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  
  tb->tick();

  CHECK("tb_ifm.precedence_increment.01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + 4),
      "Failed to update pc with the right precedence");
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

  tb_ifm_debug_during_request(tb);
  tb_ifm_debug_during_ack(tb);
  tb_ifm_debug_during_wait(tb);
  tb_ifm_debug_during_memory_stall(tb);
  tb_ifm_debug_on_output_handshake(tb);
  tb_ifm_debug_during_pipeline_stall(tb);
  tb_ifm_debug_back_to_back(tb);

  tb_ifm_interrupt_during_request(tb);
  tb_ifm_interrupt_during_ack(tb);
  tb_ifm_interrupt_during_wait(tb);
  tb_ifm_interrupt_during_memory_stall(tb);
  tb_ifm_interrupt_on_output_handshake(tb);
  tb_ifm_interrupt_during_pipeline_stall(tb);
  tb_ifm_interrupt_back_to_back(tb);

  tb_ifm_branch_during_request(tb);
  tb_ifm_branch_during_ack(tb);
  tb_ifm_branch_during_wait(tb);
  tb_ifm_branch_during_memory_stall(tb);
  tb_ifm_branch_on_output_handshake(tb);
  tb_ifm_branch_during_pipeline_stall(tb);
  tb_ifm_branch_back_to_back(tb);

  tb_ifm_precedence_debug(tb);
  tb_ifm_precedence_interrupt(tb);
  tb_ifm_precedence_branch(tb);
  tb_ifm_precedence_increment(tb);

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
