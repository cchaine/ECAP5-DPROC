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

enum CondId {
  COND_state,
  COND_input_ready,
  COND_wishbone,
  COND_register,
  COND_output_valid,
  __CondIdEnd
};

enum TestcaseId {
  T_NO_STALL_LB   =  0,
  T_NO_STALL_LBU  =  1,
  T_NO_STALL_LH   =  2,
  T_NO_STALL_LHU  =  3,
  T_NO_STALL_LW   =  4,
  T_NO_STALL_SB   =  5,
  T_NO_STALL_SH   =  6,
  T_NO_STALL_SW   =  7,
  T_MEMORY_STALL  =  8,
  T_MEMORY_WAIT   =  9,
  T_BYPASS        =  10,
  T_BUBBLE        =  11,
  T_BACK_TO_BACK  =  12
};

class TB_Lsm : public Testbench<Vtb_lsm> {
public:
  void reset() {
    this->_nop();
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_lsm>::reset();
  }

  void _nop() {
    this->core->alu_result_i = 0;
    this->core->enable_i = 0;
    this->core->write_i = 0;
    this->core->sel_i = 0x0;
    this->core->unsigned_load_i = 0;
    this->core->reg_write_i = 0;
    this->core->reg_addr_i = 0;
  }

  void _lb(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x1;
    this->core->unsigned_load_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _lbu(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x1;
    this->core->unsigned_load_i = 1;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _lh(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x3;
    this->core->unsigned_load_i = 0;
    this->core->reg_write_i = 1;
    this->core->reg_addr_i = reg_addr;
  }

  void _lw(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0xF;
    this->core->unsigned_load_i = 0;
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
  
  void _bypass(uint32_t alu_result, uint8_t write, uint8_t reg_addr) {
    this->_nop();
    this->core->enable_i = 0;
    this->core->alu_result_i = alu_result;
    this->core->reg_write_i = write;
    this->core->reg_addr_i = reg_addr;
  }
};

void tb_lsm_no_stall_lb(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_LB;

  CHECK("tb_lsm.no_stall.LB_01",
      false,
      "TODO: no_stall LB");
}

void tb_lsm_no_stall_lbu(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_LBU;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a LBU 
  //    tick 1. Acknowledge the request with reponse data and set the inputs to request a nop
  //    tick 2. Nothing (core latches response data)
  //    tick 3. Nothing (core outputs result of LBU)
  //    tick 4. Nothing (core outputs result of nop)

  //=================================
  //      Tick (0)
  
  tb->reset();

  // LH

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lbu(addr, reg_addr);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;

  tb->_nop();

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_register, (core->reg_data_o            ==  0)); 

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_lsm.no_stall.LBU_01",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);
}

void tb_lsm_no_stall_lh(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_LH;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a LH 
  //    tick 1. Acknowledge the request with reponse data and set the inputs to request a nop
  //    tick 2. Nothing (core latches response data)
  //    tick 3. Nothing (core outputs result of LH)
  //    tick 4. Nothing (core outputs result of nop)

  //=================================
  //      Tick (0)
  
  tb->reset();

  tb->check(COND_state,       (core->tb_lsm->dut->state_q  ==  0));
  tb->check(COND_input_ready, (core->input_ready_o         ==  0));
  tb->check(COND_wishbone,    (core->wb_stb_o              ==  0)    &&
                              (core->wb_cyc_o              ==  0));  

  // LH

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  1));         
  tb->check(COND_input_ready,   (core->input_ready_o         ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  addr) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  3)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_register,      (core->reg_write_o           ==  1)    &&
                                (core->reg_addr_o            ==  reg_addr));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Set inputs

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;

  tb->_nop();

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  3));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_register,      (core->reg_write_o           ==  1)    &&
                                (core->reg_addr_o            ==  reg_addr));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs

  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  0));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)        &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_register,      (core->reg_write_o           ==  1)        &&
                                (core->reg_addr_o            ==  reg_addr) &&
                                (core->reg_data_o            ==  data)); 
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));

  //`````````````````````````````````
  //      Set inputs

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  0));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  1));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_register,      (core->reg_write_o           ==  0)    &&
                                (core->reg_addr_o            ==  0)    &&
                                (core->reg_data_o            ==  0)); 
  tb->check(COND_output_valid,  (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_lsm.no_stall.LH_01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_lsm.no_stall.LH_02",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o signal", tb->err_cycles[COND_input_ready]);

  CHECK("tb_lsm.no_stall.LH_03",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.no_stall.LH_04",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_lsm.no_stall.LH_05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_no_stall_lhu(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_LHU;

  CHECK("tb_lsm.no_stall.LHU_01",
      false,
      "TODO: no_stall LHU");
}

void tb_lsm_no_stall_lw(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_LW;

  CHECK("tb_lsm.no_stall.LW_01",
      false,
      "TODO: no_stall LW");
}

void tb_lsm_no_stall_sb(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_SB;

  CHECK("tb_lsm.no_stall.SB_01",
      false,
      "TODO: no_stall SB");
}

void tb_lsm_no_stall_sh(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_SH;

  CHECK("tb_lsm.no_stall.SH_01",
      false,
      "TODO: no_stall SH");
}

void tb_lsm_no_stall_sw(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_NO_STALL_SW;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a SW
  //    tick 1. Acknowledge the request and set the inputs to request a nop
  //    tick 2. Nothing (core detects end of request)
  //    tick 3. Nothing (core doesn't output anything)

  //=================================
  //      Tick (0)
   
  tb->reset();

  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  // SW
  
  //`````````````````````````````````
  //      Set inputs

  uint32_t addr = rand();
  uint32_t data = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_sw(addr, data, reg_addr);

  //=================================
  //      Tick (0)
    
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  1));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  1));
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  addr) &&
                                (core->wb_dat_o              ==  data) &&
                                (core->wb_we_o               ==  1)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_register,      (core->reg_write_o           ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
    
  core->wb_ack_i = 1;

  tb->_nop();

  //=================================
  //      Tick (1)
    
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  3));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_register,      (core->reg_write_o           ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  0));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_register,      (core->reg_write_o           ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));

  //`````````````````````````````````
  //      Set inputs
    
  // Next instruction
  
  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,         (core->tb_lsm->dut->state_q  ==  0));
  tb->check(COND_input_ready,   (core->input_ready_o         ==  1));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_register,      (core->reg_write_o           ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_lsm.no_stall.SW_01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_lsm.no_stall.SW_02",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o signal", tb->err_cycles[COND_input_ready]);

  CHECK("tb_lsm.no_stall.SW_03",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.no_stall.SW_04",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_lsm.no_stall.SW_05",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_memory_stall(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_MEMORY_STALL;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a LH with a stalled memory
  //    tick 1. Nothing (core holds request)
  //    tick 2. Nothing (core holds request)
  //    tick 3. Acknowledge the request with reponse data and set the inputs to request a nop
  //    tick 4. Nothing (core outputs result of LH)
  //    tick 5. Nothing (core outputs result of nop)

  //=================================
  //      Tick (0)
  
  tb->reset();

  // LH

  //`````````````````````````````````
  //      Set inputs
  
  // Memory stall
  core->wb_stall_i = 1;
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  4));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  1)    &&
                               (core->wb_cyc_o              ==  1));  

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop();

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  4));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  1)    &&
                               (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  1));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  1)    &&
                               (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;

  //=================================
  //      Tick (4)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)    &&
                               (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (5)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)    &&
                               (core->wb_cyc_o              ==  0));  
  tb->check(COND_output_valid, (core->output_valid_o        ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_lsm.memory_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_lsm.memory_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.memory_stall.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_memory_wait(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_MEMORY_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a LH
  //    tick 1. Set the inputs to request a nop (core waits for response)
  //    tick 2. Nothing (core waits for response)
  //    tick 3. Nothing (core waits for response)
  //    tick 4. Acknowledge the request with reponse data
  //    tick 5. Nothing (core outputs result of LH)
  //    tick 6. Nothing (core outputs result of nop)

  //=================================
  //      Tick (0)
   
  tb->reset();

  // LH

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  tb->_nop();

  //=================================
  //      Tick (2)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  2));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)  &&
                               (core->wb_cyc_o              ==  1));
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  2));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)  &&
                               (core->wb_cyc_o              ==  1));
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_state,        (core->tb_lsm->dut->state_q  ==  2));
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)  &&
                               (core->wb_cyc_o              ==  1));
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  // Send the response

  uint32_t data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)  &&
                               (core->wb_cyc_o              ==  1));
  tb->check(COND_output_valid, (core->output_valid_o        ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o              ==  0)  &&
                               (core->wb_cyc_o              ==  0));
  tb->check(COND_output_valid, (core->output_valid_o        ==  1));

  //`````````````````````````````````
  //      Formal Checks 
    
  CHECK("tb_lsm.memory_wait.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_lsm.memory_wait.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.memory_wait.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_bypass(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_BYPASS;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a bypass with write
  //    tick 1. Nothing (core outputs bypass with write)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs

  core->input_valid_i = 1;

  uint32_t data = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_bypass(data, 1, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o        ==  0)        &&
                               (core->wb_cyc_o        ==  0));
  tb->check(COND_register,     (core->reg_write_o     ==  1)        && 
                               (core->reg_addr_o      ==  reg_addr) &&
                               (core->reg_data_o      ==  data));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_lsm.bypass.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.bypass.02",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_lsm.bypass.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_bubble(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_BUBBLE;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a bypass with write
  //    tick 1. Invalidate the input (core outputs bypass)
  //    tick 2. Nothing (core outputs bubble)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;

  uint32_t data = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_bypass(data, 1, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 0;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_wishbone,     (core->wb_stb_o        ==  0)        &&
                               (core->wb_cyc_o        ==  0));
  tb->check(COND_register,     (core->reg_write_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_lsm.bubble.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_lsm.bubble.02",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_lsm.bubble.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_lsm_back_to_back(TB_Lsm * tb) {
  Vtb_lsm * core = tb->core;
  core->testcase = T_BACK_TO_BACK;

  // The following actions are performed in this test :
  //    tick 0. Set inputs to request LH
  //    tick 1. Acknowledge the request with response data and set inputs to request bypass with write
  //    tick 2. Nothing (core latches response data)
  //    tick 3. Nothing (core outputs result of LH)
  //    tick 4. Set inputs to request SB (core outputs result of bypass)
  //    tick 5. Acknowledge the request and set inputs to resquest bypass with write
  //    tick 6. Nothing
  //    tick 7. Nothing (core outputs nothing (SB))
  //    tick 8. Nothing (core outputs result of bypass)

  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lh(addr, reg_addr);

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_register,     (core->reg_write_o     ==  1));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  reg_addr = 1 + rand() % 31;
  tb->_bypass(data, 1, reg_addr);

  data = rand();
  core->wb_ack_i = 1;
  core->wb_dat_i = data;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_register,     (core->reg_write_o     ==  1));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_ack_i = 0;
  core->wb_dat_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_register,     (core->reg_write_o     ==  1));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_register,     (core->reg_write_o     ==  1));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  addr = rand();
  data = rand();
  reg_addr = 1 + rand() % 31;
  tb->_sb(addr, data, reg_addr);

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_register,     (core->reg_write_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));
  
  //`````````````````````````````````
  //      Set inputs
  
  data = rand();
  reg_addr = 1 + rand() % 31;
  tb->_bypass(data, 1, reg_addr);

  core->wb_ack_i = 1;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_register,     (core->reg_write_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_ack_i = 0;

  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_register,     (core->reg_write_o     ==  0));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //=================================
  //      Tick (8)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 

  tb->check(COND_register,     (core->reg_write_o     ==  1));
  tb->check(COND_output_valid, (core->output_valid_o  ==  1));

  //`````````````````````````````````
  //      Formal Checks 
   
  CHECK("tb_lsm.back_to_back.01",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_lsm.back_to_back.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Lsm * tb = new TB_Lsm;
  tb->open_trace("waves/lsm.vcd");
  tb->open_testdata("testdata/lsm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_lsm_no_stall_lb(tb);
  tb_lsm_no_stall_lbu(tb);
  tb_lsm_no_stall_lh(tb);
  tb_lsm_no_stall_lhu(tb);
  tb_lsm_no_stall_lw(tb);

  tb_lsm_no_stall_sb(tb);
  tb_lsm_no_stall_sh(tb);
  tb_lsm_no_stall_sw(tb);

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
