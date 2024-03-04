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
    (core->wb_sel_o == 0xF),
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
  CHECK("tb_ifm.memory_stall_01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform the memory read request : 1st time");

  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.memory_stall_02",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 2nd time");

  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.memory_stall_03",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : 3rd time");

  core->wb_stall_i = 0;
  tb->tick();

  // check the memory read request
  CHECK("tb_ifm.memory_stall_04",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0) && (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to perform hold the memory read request : final time");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.memory_stall_05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_stall_06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.memory_stall_07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");
}

void tb_ifm_memory_wait_state(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;
  tb->tick();

  // read request is sent
  
  // insert one wait cycle
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  // insert second wait cycle
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state_03",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state_04",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  // check that the request is being processed
  CHECK("tb_ifm.memory_wait_state_05",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.memory_wait_state_06",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done and the value is outputed
  CHECK("tb_ifm.memory_wait_state_07",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");
}

void tb_ifm_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  // send a response to the read request
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  tb->tick();

  // the request is being processed

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // the response has been received

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall_01",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall_02",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall_03",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall_04",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  tb->tick();

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall_05",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall_06",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall_07",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall_08",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  core->output_ready_i = 1;

  tb->tick();

  // check that the output is valid
  CHECK("tb_ifm.pipeline_stall_09",
    (core->output_valid_o == 1),
    "Failed to validate the output");

  CHECK("tb_ifm.pipeline_stall_10",
    (core->instr_o == data),
    "Failed to output the instruction");

  CHECK("tb_ifm.pipeline_stall_11",
    (core->pc_o == Vtb_ifm_ecap5_dproc_pkg::boot_address),
    "Failed to output pc");

  // check that no memory request is performed in the meantime
  CHECK("tb_ifm.pipeline_stall_12",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to detect a pipeline stall");

  tb->tick();

  // check that a successfull output handshake is properly detected
  CHECK("tb_ifm.pipeline_stall_13",
      (core->output_valid_o == 0),
      "Failed to invalidate the output after a successfull handshake");

  // check the incremented address
  CHECK("tb_ifm.pipeline_stall_14",
    (core->wb_adr_o == (Vtb_ifm_ecap5_dproc_pkg::boot_address + 4)),
    "Failed to increment pc");
  
  // check the memory read request
  CHECK("tb_ifm.pipeline_stall_15",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1) && (core->wb_we_o == 0),
    "Failed to perform the subsequent memory request");
}

/*============================================*/
/*                   Debug                    */
/*============================================*/

void tb_ifm_debug_after_reset(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.debug_after_reset_01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_debug_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.debug_during_ack_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.debug_during_ack_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.debug_during_ack_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_during_ack_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_ack_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_debug_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.debug_during_wait_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_during_wait_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_wait_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_debug_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  CHECK("tb_ifm.debug_during_memory_stall_01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");

  // check the memory read request is updated
  CHECK("tb_ifm.debug_during_memory_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to update the memory read request address");
}

void tb_ifm_debug_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.debug_on_output_handshake_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_on_output_handshake_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_on_output_handshake_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_debug_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  // check the stage output
  CHECK("tb_ifm.debug_during_pipeline_stall_01",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_during_pipeline_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_debug_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.debug_back_to_back_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.debug_back_to_back_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->drq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.debug_back_to_back_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.debug_back_to_back_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.debug_back_to_back_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

/*============================================*/
/*                 Interrupt                  */
/*============================================*/

void tb_ifm_interrupt_after_reset(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.interrupt_after_reset_01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.interrupt_during_ack_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.interrupt_during_ack_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.interrupt_during_ack_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_during_ack_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_ack_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.interrupt_during_wait_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_during_wait_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_wait_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  CHECK("tb_ifm.interrupt_during_memory_stall_01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");

  // check the memory read request is updated
  CHECK("tb_ifm.interrupt_during_memory_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to update the memory read request address");
}

void tb_ifm_interrupt_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.interrupt_on_output_handshake_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_on_output_handshake_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_on_output_handshake_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  // check the stage output
  CHECK("tb_ifm.interrupt_during_pipeline_stall_01",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_during_pipeline_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_interrupt_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.interrupt_back_to_back_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.interrupt_back_to_back_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->irq_i = 0;

  // check that the request is done
  CHECK("tb_ifm.interrupt_back_to_back_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.interrupt_back_to_back_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.interrupt_back_to_back_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address),
      "Failed to fetch the interruption instruction");
}

/*============================================*/
/*                   Branch                   */
/*============================================*/

void tb_ifm_branch_after_reset(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.branch_after_reset_01",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.branch_during_ack_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.branch_during_ack_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();

  // check that the request is done
  CHECK("tb_ifm.branch_during_ack_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_during_ack_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_ack_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.branch_during_wait_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_during_wait_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_wait_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  // check the memory read request is updated
  CHECK("tb_ifm.branch_during_memory_stall_01",
    (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
    "Failed to perform hold the memory read request");

  CHECK("tb_ifm.branch_during_memory_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to update the memory read request address");
}

void tb_ifm_branch_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  CHECK("tb_ifm.branch_on_output_handshake_01",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_on_output_handshake_02",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_on_output_handshake_03",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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

  // check the stage output
  CHECK("tb_ifm.branch_during_pipeline_stall_01",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_during_pipeline_stall_02",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
}

void tb_ifm_branch_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
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
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;

  // check that the request is being processed
  CHECK("tb_ifm.branch_back_to_back_01",
      (core->wb_stb_o == 0),
      "Failed to deassert stb after issuing the memory read request");
  CHECK("tb_ifm.branch_back_to_back_02",
      (core->wb_cyc_o == 1),
      "Failed to maintain cyc for the entire duration of the read request");

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  tb->tick();
  core->branch_i = 0;

  // check that the request is done
  CHECK("tb_ifm.branch_back_to_back_03",
    (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
    "Failed to end the memory read request");

  // check the stage output
  CHECK("tb_ifm.branch_back_to_back_04",
    (core->output_valid_o == 0),
    "Failed to cancel output");
  
  tb->tick();

  CHECK("tb_ifm.branch_back_to_back_05",
      (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + core->boffset_i),
      "Failed to fetch the interruption instruction");
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

  tb_ifm_debug_after_reset(tb);
  tb_ifm_debug_during_ack(tb);
  tb_ifm_debug_during_wait(tb);
  tb_ifm_debug_during_memory_stall(tb);
  tb_ifm_debug_on_output_handshake(tb);
  tb_ifm_debug_during_pipeline_stall(tb);
  tb_ifm_debug_back_to_back(tb);

  tb_ifm_interrupt_after_reset(tb);
  tb_ifm_interrupt_during_ack(tb);
  tb_ifm_interrupt_during_wait(tb);
  tb_ifm_interrupt_during_memory_stall(tb);
  tb_ifm_interrupt_on_output_handshake(tb);
  tb_ifm_interrupt_during_pipeline_stall(tb);
  tb_ifm_interrupt_back_to_back(tb);

  tb_ifm_branch_after_reset(tb);
  tb_ifm_branch_during_ack(tb);
  tb_ifm_branch_during_wait(tb);
  tb_ifm_branch_during_memory_stall(tb);
  tb_ifm_branch_on_output_handshake(tb);
  tb_ifm_branch_during_pipeline_stall(tb);
  tb_ifm_branch_back_to_back(tb);

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
