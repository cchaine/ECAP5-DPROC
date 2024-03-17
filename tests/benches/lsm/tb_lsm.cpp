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

#include "Vtb_lsm.h"
#include "testbench.h"
#include "Vtb_lsm_ecap5_dproc_pkg.h"

class TB_Lsm : public Testbench<Vtb_lsm> {
public:
  void reset() {
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

void tb_lsm_no_stall_lb(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();

  core->wb_stall_i = 0;
}

void tb_lsm_no_stall_lh(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();

  core->wb_stall_i = 0;
}

void tb_lsm_no_stall_lw(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();

  core->wb_stall_i = 0;
}

void tb_lsm_no_stall_sb(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_no_stall_sh(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_no_stall_sw(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_memory_stall_load(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_memory_stall_store(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_memory_wait_load(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_memory_wait_store(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_bypass(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
}

void tb_lsm_bubble(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  tb->reset();
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

  tb_lsm_no_stall_lb(tb);
  tb_lsm_no_stall_lh(tb);
  tb_lsm_no_stall_lw(tb);

  tb_lsm_no_stall_sb(tb);
  tb_lsm_no_stall_sh(tb);
  tb_lsm_no_stall_sw(tb);

  tb_lsm_memory_stall_load(tb);
  tb_lsm_memory_stall_store(tb);

  tb_lsm_memory_wait_load(tb);
  tb_lsm_memory_wait_store(tb);

  tb_lsm_bypass(tb);
  tb_lsm_bubble(tb);

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
