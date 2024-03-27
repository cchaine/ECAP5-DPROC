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

#include "testbench.h"

#include "Vtb_lsm.h"
#include "Vtb_lsm_tb_lsm.h"
#include "Vtb_lsm_lsm.h"
#include "Vtb_lsm_ecap5_dproc_pkg.h"

class TB_Lsm : public Testbench<Vtb_lsm> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;
  }

  void _nop() {
    this->core->alu_result_i = 0;
    this->core->enable_i = 0;
    this->core->write_i = 0;
    this->core->sel_i = 0x0;
    this->core->reg_write_i = 0;
    this->core->reg_addr_i = 0;
  }

  void _lb(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x1;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _lh(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x3;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _lw(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0xF;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _sb(uint32_t addr, uint32_t data, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 1;
    this->core->write_data_i = data;
    this->core->sel_i = 0x1;
    this->core->reg_write_i = 0;
  }

  void _sh(uint32_t addr, uint32_t data, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 1;
    this->core->write_data_i = data;
    this->core->sel_i = 0x3;
    this->core->reg_write_i = 0;
  }

  void _sw(uint32_t addr, uint32_t data, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 1;
    this->core->write_data_i = data;
    this->core->sel_i = 0xF;
    this->core->reg_write_i = 0;
  }
};

void tb_lsm_no_stall_load(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 1;
  tb->reset();

  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  // During reset

  CHECK("tb_lsm.no_stall.LOAD_01",
      (core->tb_lsm->dut->state_q == 0),
      "Failed to reset to the IDLE state");

  CHECK("tb_lsm.no_stall.LOAD_28",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to initialize the memory interface");

  // LH

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);
  tb->tick();
  tb->_nop();

  CHECK("tb_lsm.no_stall.LOAD_04",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.no_stall.LOAD_05",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_o");

  CHECK("tb_lsm.no_stall.LOAD_06",
      (core->tb_lsm->dut->state_q == 1),
      "Failed to switch to the REQUEST state");

  CHECK("tb_lsm.no_stall.LOAD_07",
      (core->input_ready_o == 1),
      "Failed to keep input_ready_o asserted input_ready_o");

  CHECK("tb_lsm.no_stall.LOAD_08",
      (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
      "Failed to trigger the memory request");

  CHECK("tb_lsm.no_stall.LOAD_09",
      (core->wb_adr_o == addr) && (core->wb_we_o == 0),
      "Failed to trigger a memory read request");

  CHECK("tb_lsm.no_stall.LOAD_10",
      (core->wb_sel_o == 0x3),
      "Failed to set the wishbone sel signal");

  CHECK("tb_lsm.no_stall.LOAD_11",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  // Answer memory request

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;
  tb->tick();

  CHECK("tb_lsm.no_stall.LOAD_12",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.no_stall.LOAD_13",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_o");

  CHECK("tb_lsm.no_stall.LOAD_14",
      (core->tb_lsm->dut->state_q == 3),
      "Failed to switch to the DONE state");

  CHECK("tb_lsm.no_stall.LOAD_15",
      (core->input_ready_o == 0),
      "Failed to deassert input_ready_o");

  CHECK("tb_lsm.no_stall.LOAD_16",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  CHECK("tb_lsm.no_stall.LOAD_29",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to detect a successfull memory request");

  core->wb_ack_i = 0;
  core->wb_dat_i = 0;
  tb->tick();

  // Output the result

  CHECK("tb_lsm.no_stall.LOAD_17",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.no_stall.LOAD_18",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_o");

  CHECK("tb_lsm.no_stall.LOAD_19",
      (core->tb_lsm->dut->state_q == 0),
      "Failed to switch to the IDLE state");

  CHECK("tb_lsm.no_stall.LOAD_20",
      (core->input_ready_o == 0),
      "Failed to deassert input_ready_o");

  CHECK("tb_lsm.no_stall.LOAD_21",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.no_stall.LOAD_22",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  CHECK("tb_lsm.no_stall.LOAD_30",
      (core->wb_cyc_o == 0),
      "Failed to deasssert cyc after successfull memory request");

  // Next instruction (nop)

  tb->tick();

  CHECK("tb_lsm.no_stall.LOAD_23",
      (core->input_ready_o == 1),
      "Failed to assert input_ready_o");

  CHECK("tb_lsm.no_stall.LOAD_24",
      (core->output_valid_o == 1),
      "Failed to invalidate the output");

  CHECK("tb_lsm.no_stall.LOAD_25",
      (core->reg_write_o == 0),
      "Failed to deassert reg_write_o");
  
  CHECK("tb_lsm.no_stall.LOAD_26",
      (core->reg_addr_o == 0),
      "Failed to set reg_addr_o");

  CHECK("tb_lsm.no_stall.LOAD_27",
      (core->reg_data_o == 0),
      "Failed to set reg_data_o");
}

void tb_lsm_no_stall_store(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 2;
  tb->reset();

  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  // SW

  uint32_t addr = rand();
  uint32_t data = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_sw(addr, data, reg_addr);
  tb->tick();
  tb->_nop();

  CHECK("tb_lsm.no_stall.STORE_01",
      (core->reg_write_o == 0),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.no_stall.STORE_02",
      (core->tb_lsm->dut->state_q == 1),
      "Failed to switch to the REQUEST state");

  CHECK("tb_lsm.no_stall.STORE_03",
      (core->input_ready_o == 1),
      "Failed to keep asserted input_ready_o");

  CHECK("tb_lsm.no_stall.STORE_04",
      (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
      "Failed to trigger the memory request");

  CHECK("tb_lsm.no_stall.STORE_05",
      (core->wb_adr_o == addr) && (core->wb_we_o == 1),
      "Failed to trigger a memory write request");

  CHECK("tb_lsm.no_stall.STORE_06",
      (core->wb_dat_o == core->write_data_i),
      "Failed to set the memory write data");

  CHECK("tb_lsm.no_stall.STORE_07",
      (core->wb_sel_o == 0xF),
      "Failed to set the wishbone sel signal");

  CHECK("tb_lsm.no_stall.STORE_08",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  // Answer memory request

  core->wb_ack_i = 1;
  tb->tick();

  CHECK("tb_lsm.no_stall.STORE_10",
      (core->tb_lsm->dut->state_q == 3),
      "Failed to switch to the DONE state");

  CHECK("tb_lsm.no_stall.STORE_11",
      (core->input_ready_o == 0),
      "Failed to deassert input_ready_o");

  CHECK("tb_lsm.no_stall.STORE_12",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  CHECK("tb_lsm.no_stall.STORE_19",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to detect a successfull memory request");

  core->wb_ack_i = 0;
  core->wb_dat_i = 0;
  tb->tick();

  // Output the result (none)

  CHECK("tb_lsm.no_stall.STORE_14",
      (core->tb_lsm->dut->state_q == 0),
      "Failed to switch to the IDLE state");

  CHECK("tb_lsm.no_stall.STORE_15",
      (core->input_ready_o == 0),
      "Failed to deassert input_ready_o");

  CHECK("tb_lsm.no_stall.STORE_16",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();

  // Next instruction

  CHECK("tb_lsm.no_stall.STORE_17",
      (core->input_ready_o == 1),
      "Failed to assert input_ready_o");

  CHECK("tb_lsm.no_stall.STORE_18",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  CHECK("tb_lsm.no_stall.STORE_20",
      (core->wb_cyc_o == 0),
      "Failed to deasssert cyc after successfull memory request");
}

void tb_lsm_memory_stall(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 3;
  tb->reset();

  // Memory stall
  core->wb_stall_i = 1;
  core->input_valid_i = 1;

  // LH

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);
  tb->tick();
  tb->_nop();

  // Hold the request
  
  CHECK("tb_lsm.memory_stall.01",
      (core->tb_lsm->dut->state_q == 4),
      "Failed to switch to the MEMORY_STALL state");

  CHECK("tb_lsm.memory_stall.02",
      (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
      "Failed to hold the memory request");

  tb->tick();

  // Hold the request
  
  CHECK("tb_lsm.memory_stall.03",
      (core->tb_lsm->dut->state_q == 4),
      "Failed to stay in the MEMORY_STALL state");

  CHECK("tb_lsm.memory_stall.04",
      (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_stall.05",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  core->wb_stall_i = 0;
  tb->tick();

  // Hold the request

  CHECK("tb_lsm.memory_stall.06",
      (core->wb_stb_o == 1) && (core->wb_cyc_o == 1),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_stall.07",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  // Answer to the request

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;
  tb->tick();
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  CHECK("tb_lsm.memory_stall.08",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_stall.09",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  tb->tick();

  // Output the result

  CHECK("tb_lsm.memory_stall.10",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_stall.11",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();

  // Next instruction
}

void tb_lsm_memory_wait(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 4;
  tb->reset();

  core->input_valid_i = 1;

  // LH

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);
  tb->tick();
  tb->_nop();

  tb->tick();
  
  // Wait before sending the response
  
  CHECK("tb_lsm.memory_wait.01",
      (core->tb_lsm->dut->state_q == 2),
      "Failed to switch to the MEMORY_WAIT state");

  CHECK("tb_lsm.memory_wait.02",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to wait for the memory response");

  CHECK("tb_lsm.memory_wait.03",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  tb->tick();

  // Wait before sending the response
  
  CHECK("tb_lsm.memory_wait.04",
      (core->tb_lsm->dut->state_q == 2),
      "Failed to stay in the MEMORY_WAIT state");

  CHECK("tb_lsm.memory_wait.05",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to wait for the memory response");

  CHECK("tb_lsm.memory_wait.06",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  tb->tick();

  // Wait before sending the response
  
  CHECK("tb_lsm.memory_wait.07",
      (core->tb_lsm->dut->state_q == 2),
      "Failed to stay in the MEMORY_WAIT state");

  CHECK("tb_lsm.memory_wait.08",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to wait for the memory response");

  CHECK("tb_lsm.memory_wait.09",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  // Send the response

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;
  tb->tick();
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  CHECK("tb_lsm.memory_wait.10",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 1),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_wait.11",
      (core->output_valid_o == 0),
      "Failed to validate the output");

  tb->tick();

  // Output the result

  CHECK("tb_lsm.memory_wait.12",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to hold the memory request");

  CHECK("tb_lsm.memory_wait.13",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();
  
  // Next instruction
}

void tb_lsm_bypass(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 5;
  tb->reset();

  core->input_valid_i = 1;

  // Bypass
  core->enable_i = 0;
  core->reg_write_i = 1;
  uint32_t data = rand();
  core->alu_result_i = data;
  uint32_t reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;
  tb->tick();
  tb->_nop();

  CHECK("tb_lsm.bypass.01",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.bypass.02",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_i");

  CHECK("tb_lsm.bypass.03",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.bypass.04",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.bypass.05",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();
}

void tb_lsm_bubble(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 6;
  tb->reset();

  core->input_valid_i = 1;

  // Bypass

  core->enable_i = 0;
  core->reg_write_i = 1;
  uint32_t data = rand();
  core->alu_result_i = data;
  uint32_t reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;
  tb->tick();

  CHECK("tb_lsm.bubble.01",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.bubble.02",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_i");

  CHECK("tb_lsm.bubble.03",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.bubble.04",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.bubble.05",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  core->input_valid_i = 0;
  tb->tick();

  // Bubble

  CHECK("tb_lsm.bubble.06",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.bubble.07",
      (core->reg_write_o == 0),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.bubble.08",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  // Bubble

  tb->tick();

  CHECK("tb_lsm.bubble.09",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.bubble.10",
      (core->reg_write_o == 0),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.bubble.11",
      (core->output_valid_o == 1),
      "Failed to validate the output");
}

void tb_lsm_back_to_back(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = 7;
  tb->reset();
  tb->_nop();

  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  // LH

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);
  tb->tick();

  CHECK("tb_lsm.back_to_back.01",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  CHECK("tb_lsm.back_to_back.02",
      (core->reg_write_o == 1),
      "Failed to assert reg_write_o");

  // Bypass

  core->enable_i = 0;
  core->reg_write_i = 1;
  uint32_t data = rand();
  core->alu_result_i = data;
  reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;

  // Send the response to LH

  data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;
  tb->tick();
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  CHECK("tb_lsm.back_to_back.03",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  CHECK("tb_lsm.back_to_back.04",
      (core->reg_write_o == 1),
      "Failed to deassert reg_write_o");

  tb->tick();

  // Output the result of LH

  CHECK("tb_lsm.back_to_back.05",
      (core->output_valid_o == 1),
      "Failed to invalidate the output");

  CHECK("tb_lsm.back_to_back.06",
      (core->reg_write_o == 1),
      "Failed to deassert reg_write_o");

  tb->tick();

  // Output the result of the bypass

  CHECK("tb_lsm.back_to_back.07",
      (core->reg_write_o == 1),
      "Failed to assert reg_write_o");

  CHECK("tb_lsm.back_to_back.08",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  // SB

  addr = rand();
  data = rand();
  reg_addr = 1 + rand() % 31;
  tb->_sb(addr, data, reg_addr);
  tb->tick();

  // Bypass

  core->enable_i = 0;
  core->reg_write_i = 1;
  data = rand();
  core->alu_result_i = data;
  reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;

  CHECK("tb_lsm.back_to_back.09",
      (core->reg_write_o == 0),
      "Failed to deassert reg_write_o");

  CHECK("tb_lsm.back_to_back.10",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  // Answer to SB

  core->wb_ack_i = 1;
  tb->tick();
  core->wb_ack_i = 0;

  CHECK("tb_lsm.back_to_back.11",
      (core->reg_write_o == 0),
      "Failed to deassert reg_write_o");

  CHECK("tb_lsm.back_to_back.12",
      (core->output_valid_o == 0),
      "Failed to invalidate the output");

  tb->tick();

  // Output the result of SB (none)
  
  CHECK("tb_lsm.back_to_back.13",
      (core->reg_write_o == 0),
      "Failed to deassert reg_write_o");

  CHECK("tb_lsm.back_to_back.14",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  tb->tick();
  
  // Output the result of bypass

  CHECK("tb_lsm.back_to_back.15",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.back_to_back.16",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_i");

  CHECK("tb_lsm.back_to_back.17",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.back_to_back.18",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.back_to_back.19",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  // Bypass

  core->enable_i = 0;
  core->reg_write_i = 1;
  data = rand();
  core->alu_result_i = data;
  reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;
  tb->tick();

  // Output the result of bypass

  CHECK("tb_lsm.back_to_back.20",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.back_to_back.21",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_i");

  CHECK("tb_lsm.back_to_back.22",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.back_to_back.23",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.back_to_back.24",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  // Bypass

  core->enable_i = 0;
  core->reg_write_i = 1;
  data = rand();
  core->alu_result_i = data;
  reg_addr = 1 + rand() % 31;
  core->reg_addr_i = reg_addr;
  tb->tick();

  // Output the result of bypass

  CHECK("tb_lsm.back_to_back.25",
      (core->reg_write_o == 1),
      "Failed to set reg_write_o");

  CHECK("tb_lsm.back_to_back.26",
      (core->reg_addr_o == reg_addr),
      "Failed to set reg_addr_i");

  CHECK("tb_lsm.back_to_back.27",
      (core->reg_data_o == data),
      "Failed to set reg_data_o");

  CHECK("tb_lsm.back_to_back.28",
      (core->wb_stb_o == 0) && (core->wb_cyc_o == 0),
      "Failed to not trigger a memory request");

  CHECK("tb_lsm.back_to_back.29",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  // Bypass without reg write
  tb->_nop();
  tb->tick();

  // Output the result of the bypass (none)

  CHECK("tb_lsm.back_to_back.30",
      (core->output_valid_o == 1),
      "Failed to validate the output");

  CHECK("tb_lsm.back_to_back.31",
      (core->reg_write_o == 0),
      "Failed to deassert reg_write_o");
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Lsm * tb = new TB_Lsm;
  tb->open_trace("waves/lsm.vcd");
  tb->open_testdata("testdata/lsm.csv");
  tb->set_debug_log(verbose);

  /************************************************************/

  tb_lsm_no_stall_load(tb);
  tb_lsm_no_stall_store(tb);

  tb_lsm_memory_stall(tb);

  tb_lsm_memory_wait(tb);

  tb_lsm_bypass(tb);
  tb_lsm_bubble(tb);

  tb_lsm_back_to_back(tb);

  /************************************************************/

  printf("[LSM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
