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

enum CondId {
  COND_s1_stall,
  COND_s1_ack,
  COND_s2_stall,
  COND_s2_ack,
  COND_m_wb,
  __CondIdEnd
};

enum TestcaseId {
  T_PORT1_READ      =  1,
  T_PORT1_WRITE     =  2,
  T_PORT2_READ      =  3,
  T_PORT2_WRITE     =  4,
  T_TWO_DURING_ONE  =  5,
  T_ONE_DURING_TWO  =  6,
  T_PRIORITY        =  7,
  T_MASTER_STALL_S1 =  8,
  T_MASTER_STALL_S2 =  9,
  T_BACK_TO_BACK    =  10
};

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
    this->_nop_master();
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

  void _nop_master() {
    this->core->m_wb_dat_i = 0;
    this->core->m_wb_ack_i = 0;
    this->core->m_wb_stall_i = 0;
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

void tb_emm_port1_read(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_PORT1_READ;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 1
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core ends request)
  //    tick 4. Nothing (Nothing)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->read_request_port1(addr, sel);
  tb->_nop_port2();

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.port1_read.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.port1_read.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.port1_read.03",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.port1_read.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_port1_write(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_PORT1_WRITE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for write on port 1
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core ends request)
  //    tick 4. Nothing (Nothing)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint32_t data = rand();
  uint8_t sel = 0x3;
  tb->write_request_port1(addr, data, sel);
  tb->_nop_port2();

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s1_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s1_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s1_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.port1_write.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.port1_write.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.port1_write.03",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.port1_write.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_port2_read(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_PORT2_READ;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 2
  //    tick 1. Hold inputs for read on port 2 (core unstalls port2)
  //    tick 2. Acknowledge request with response data (core makes request)
  //    tick 3. Nothing (core latches response)
  //    tick 4. Nothing (core ends request)
  //    tick 5. Nothing (core stalls port 2)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->_nop_port1();
  tb->read_request_port2(addr, sel);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.port2_read.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.port2_read.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.port2_read.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.port2_read.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_port2_write(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_PORT2_WRITE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for write on port 2
  //    tick 1. Hold inputs for write on port 2 (core unstalls port2)
  //    tick 2. Acknowledge request (core makes request)
  //    tick 3. Nothing (core latches response)
  //    tick 4. Nothing (core ends request)
  //    tick 5. Nothing (core stalls port 2)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint32_t data = rand();
  uint8_t sel = 0x3;
  tb->_nop_port1();
  tb->write_request_port2(addr, data, sel);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s2_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s2_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s2_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->m_wb_dat_o == core->s2_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.port2_write.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.port2_write.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.port2_write.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.port2_write.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_two_during_one(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_TWO_DURING_ONE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 1
  //    tick 1. Acknowledge request and set inputs for read on port 2 (port 1 request performed)
  //    tick 2. Nothing (port 1 latches response)
  //    tick 3. End port1 request
  //    tick 4. Hold port 2 request (ports switching)
  //    tick 5. Acknowledge request (port 2 request selected)
  //    tick 6. Nothing (port 2 latches response)
  //    tick 7. End port 2 request
  //    tick 8. Nothing (ports switching)

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

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  addr = rand();
  sel = rand();
  tb->read_request_port2(addr, sel);

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (6)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (7)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (8)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.two_during_one.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.two_during_one.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.two_during_one.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.two_during_one.04",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.two_during_one.05",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_one_during_two(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_ONE_DURING_TWO;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 2
  //    tick 1. Hold request on port 2 (ports switching)
  //    tick 2. Acknowledge request and set inputs for read on port 1 (port 2 request performed)
  //    tick 3. Nothing (port 2 latches response)
  //    tick 4. End port2 request
  //    tick 5. Hold port 1 request (ports switching)
  //    tick 6. Acknowledge request (port 1 request selected)
  //    tick 7. Nothing (port 1 latches response)
  //    tick 8. End port 1 request
  //    tick 9. Nothing (ports switching)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->_nop_port1();
  tb->read_request_port2(addr, sel);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  addr = rand();
  sel = rand();
  tb->read_request_port1(addr, sel);

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //=================================
  //      Tick (6)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (7)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (8)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (9)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.one_during_two.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.one_during_two.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.one_during_two.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.one_during_two.04",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.one_during_two.05",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_priority(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_PRIORITY;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 1 and port 2
  //    tick 1. Acknowledge request (port 1 request performed)
  //    tick 2. Nothing (port 1 latches response)
  //    tick 3. End port1 request
  //    tick 4. Hold port 2 request (ports switching)
  //    tick 5. Acknowledge request (port 2 request selected)
  //    tick 6. Nothing (port 2 latches response)
  //    tick 7. End port 2 request
  //    tick 8. Nothing (ports switching)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->read_request_port1(addr, sel);
  addr = rand();
  sel = 0xF;
  tb->read_request_port2(addr, sel);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (6)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (7)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (8)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.priority.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.priority.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.priority.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.priority.04",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.priority.05",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_master_stall_s1(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_MASTER_STALL_S1;

  // The following actions are performed in this test :
  //    tick 0. Stall the interface
  //    tick 1. Set inputs for s1 request
  //    tick 2. Hold inputs for s1 request and unstall interface
  //    tick 3. Acknowledge request with response data (core makes request)
  //    tick 4. Nothing (core latches response)
  //    tick 5. Nothing (core ends request)
  //    tick 6. Nothing (Nothing)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_stall_i = 1;

  //=================================
  //      Tick (1)

  tb->tick();

  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->read_request_port1(addr, sel);
  tb->_nop_port2();

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));


  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_stall_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 

  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (6)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.master_stall_s1.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.master_stall_s1.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.master_stall_s1.03",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.master_stall_s1.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_master_stall_s2(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_MASTER_STALL_S2;

  // The following actions are performed in this test :
  //    tick 0. Stall the interface
  //    tick 1. Set inputs for s2 request
  //    tick 2. Hold inputs for s2 request and unstall interface
  //    tick 3. Acknowledge request with response data (core makes request)
  //    tick 4. Nothing (core latches response)
  //    tick 5. Nothing (core ends request)
  //    tick 6. Nothing (Nothing)

  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_stall_i = 1;

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->_nop_port1();
  tb->read_request_port2(addr, sel);

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_stall_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (6)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));

  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.master_stall_s2.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.master_stall_s2.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.master_stall_s2.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.master_stall_s2.04",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
}

void tb_emm_back_to_back(TB_Emm * tb) {
  Vtb_emm * core = tb->core;
  core->testcase = T_BACK_TO_BACK;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for read on port 1
  //    tick 1. Acknowledge request and set inputs for read on port 2 (port 1 request performed)
  //    tick 2. Nothing (port 1 latches response)
  //    tick 3. End port1 request
  //    tick 4. Set inputs for read on port 1 and hold port 2 request (ports switching)
  //    tick 5. Acknowledge request (port 2 request selected)
  //    tick 6. Nothing (port 2 latches response)
  //    tick 7. End port 2 request
  //    tick 8. Nothing (ports switching)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t addr = rand();
  uint8_t sel = 0x3;
  tb->read_request_port1(addr, sel);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  addr = rand();
  sel = 0xF;
  tb->read_request_port2(addr, sel);

  uint32_t data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (2)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (3)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  addr = rand();
  sel = 0xF;
  tb->read_request_port1(addr, sel);

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port2();
  core->s2_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (6)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (7)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 1));
  tb->check(COND_s1_ack, (core->s1_wb_ack_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s2_wb_adr_i)  &&
                       (core->s2_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s2_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s2_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s2_wb_stb_i)  &&
                       (core->s2_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s2_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs

  core->s2_wb_cyc_i = 0;

  //=================================
  //      Tick (8)

  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //=================================
  //      Tick (9)

  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (10)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (11)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (12)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  addr = rand();
  sel = 0x1;
  tb->read_request_port1(addr, sel);

  //=================================
  //      Tick (13)

  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop_port1();
  core->s1_wb_cyc_i = 1;

  data = rand();
  core->m_wb_dat_i = data;
  core->m_wb_ack_i = 1;

  //=================================
  //      Tick (14)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->m_wb_dat_i = 0;
  core->m_wb_ack_i = 0;

  //=================================
  //      Tick (15)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));

  //`````````````````````````````````
  //      Set inputs
  
  core->s1_wb_cyc_i = 0;

  //=================================
  //      Tick (16)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_s1_stall, (core->s1_wb_stall_o == 0));
  tb->check(COND_s2_stall, (core->s2_wb_stall_o == 1));
  tb->check(COND_s2_ack, (core->s2_wb_ack_o == 0));
  tb->check(COND_m_wb, (core->m_wb_adr_o == core->s1_wb_adr_i)  &&
                       (core->s1_wb_dat_o == core->m_wb_dat_i)  &&
                       (core->m_wb_we_o == core->s1_wb_we_i)    &&
                       (core->m_wb_sel_o == core->s1_wb_sel_i)  &&
                       (core->m_wb_stb_o == core->s1_wb_stb_i)  &&
                       (core->s1_wb_ack_o == core->m_wb_ack_i)  &&
                       (core->m_wb_cyc_o == core->s1_wb_cyc_i));
  
  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_emm.back_to_back.01",
      tb->conditions[COND_s1_stall],
      "Failed to implement s1 stalling", tb->err_cycles[COND_s1_stall]);
  
  CHECK("tb_emm.back_to_back.02",
      tb->conditions[COND_s2_stall],
      "Failed to implement s2 stalling", tb->err_cycles[COND_s2_stall]);

  CHECK("tb_emm.back_to_back.03",
      tb->conditions[COND_s1_ack],
      "Failed to block s1 acknowledge", tb->err_cycles[COND_s1_ack]);

  CHECK("tb_emm.back_to_back.04",
      tb->conditions[COND_s2_ack],
      "Failed to block s2 acknowledge", tb->err_cycles[COND_s2_ack]);

  CHECK("tb_emm.back_to_back.05",
      tb->conditions[COND_m_wb],
      "Failed to implement master muxing", tb->err_cycles[COND_m_wb]);
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

  tb_emm_two_during_one(tb);
  tb_emm_one_during_two(tb);

  tb_emm_priority(tb);

  tb_emm_master_stall_s1(tb);
  tb_emm_master_stall_s2(tb);

  tb_emm_back_to_back(tb);

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
