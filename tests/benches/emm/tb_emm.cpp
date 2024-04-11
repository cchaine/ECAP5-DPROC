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

#include "Vtb_emm.h"
#include "testbench.h"
#include "Vtb_emm_ecap5_dproc_pkg.h"

class TB_Emm : public Testbench<Vtb_emm> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_emm>::reset();
  }
  
  void _nop() {
    this->_nop_port1();
    this->_nop_port2();
  }
  
  void _nop_port1() {
    this->core->s1_wb_adr_i = 0;
    this->core->s1_wb_dat_i = 0;
    this->core->s1_wb_we_i  = 0;
    this->core->s1_wb_sel_i = 0;
    this->core->s1_wb_stb_i = 0;
    this->core->s1_wb_cyc_i = 0;
  }

  void _nop_port2() {
    this->core->s2_wb_adr_i = 0;
    this->core->s2_wb_dat_i = 0;
    this->core->s2_wb_we_i  = 0;
    this->core->s2_wb_sel_i = 0;
    this->core->s2_wb_stb_i = 0;
    this->core->s2_wb_cyc_i = 0;
  }

  void read_request_port1(uint32_t addr, uint8_t sel) {
    this->core->s1_wb_adr_i = addr;
    this->core->s1_wb_we_i = 0;
    this->core->s1_wb_sel_i = sel;
    this->core->s1_wb_stb_i = 1;
    this->core->s1_wb_cyc_i = 1;
  }

  void read_request_port2(uint32_t addr, uint8_t sel) {
    this->core->s2_wb_adr_i = addr;
    this->core->s2_wb_we_i = 0;
    this->core->s2_wb_sel_i = sel;
    this->core->s2_wb_stb_i = 1;
    this->core->s2_wb_cyc_i = 1;
  }

  void write_request_port1(uint32_t addr, uint32_t data, uint8_t sel) {
    this->core->s1_wb_adr_i = addr;
    this->core->s1_wb_dat_i = data;
    this->core->s1_wb_we_i = 1;
    this->core->s1_wb_sel_i = sel;
    this->core->s1_wb_stb_i = 1;
    this->core->s1_wb_cyc_i = 1;
  }

  void write_request_port2(uint32_t addr, uint32_t data, uint8_t sel) {
    this->core->s2_wb_adr_i = addr;
    this->core->s2_wb_dat_i = data;
    this->core->s2_wb_we_i = 1;
    this->core->s2_wb_sel_i = sel;
    this->core->s2_wb_stb_i = 1;
    this->core->s2_wb_cyc_i = 1;
  }
};

enum CondId {
  __CondIdEnd
};

void tb_emm_port1_read(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 1;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 1
  //    tick 1. 

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->read_request_port1(addr, sel);
  tb->_nop_port2();

  //=================================
  //      Tick (1)

  tb->tick();
}

void tb_emm_port1_write(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 2;

  tb->reset();
}

void tb_emm_port2_read(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 3;

  tb->reset();
}

void tb_emm_port2_write(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 4;

  tb->reset();
}

void tb_emm_parallel(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 5;

  tb->reset();
}

void tb_emm_parallel_multiple(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 6;

  tb->reset();
}

void tb_emm_priority(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = 7;

  tb->reset();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Emm * tb = new TB_Emm;
  tb->open_trace("waves/emm.vcd");
  tb->open_testdata("testdata/emm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_emm_port1_read(tb);
  tb_emm_port1_write(tb);

  tb_emm_port2_read(tb);
  tb_emm_port2_write(tb);

  tb_emm_parallel(tb);
  tb_emm_parallel_multiple(tb);

  tb_emm_priority(tb);

  /************************************************************/

  printf("[EMM]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
