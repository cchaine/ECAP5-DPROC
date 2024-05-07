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

#include "Vtb_loadstore_w_slave.h"
#include "Vtb_loadstore_w_slave_tb_loadstore_w_slave.h"
#include "Vtb_loadstore_w_slave_ecap5_dproc_pkg.h"

enum CondId {
  COND_input_ready,
  COND_wishbone,
  COND_register,
  COND_output_valid,
  __CondIdEnd
};

enum TestcaseId {
  T_NO_STALL_LW   =  1,
  T_NO_STALL_SW   =  2
};

class TB_Loadstore_w_slave : public Testbench<Vtb_loadstore_w_slave> {
public:
  void reset() {
    this->_nop();
    this->core->input_valid_i = 0;

    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_loadstore_w_slave>::reset();
  }

  void _nop() {
    this->core->alu_result_i = 0;
    this->core->enable_i = 0;
    this->core->write_i = 0;
    this->core->sel_i = 0x0;
    this->core->write_data_i = 0;
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

  void _lhu(uint32_t addr, uint8_t reg_addr) {
    this->_nop();
    this->core->alu_result_i = addr;
    this->core->enable_i = 1;
    this->core->write_i = 0;
    this->core->sel_i = 0x3;
    this->core->unsigned_load_i = 1;
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

  uint32_t sign_extend(uint32_t data, uint32_t nb_bits) {
    data &= (1 << nb_bits)-1;
    if((data >> (nb_bits-1)) & 0x1){
      data |= (((1 << (32 - (nb_bits-1))) - 1) << nb_bits);
    }
    return data;
  }
};

void tb_loadstore_w_slave_no_stall_lw(TB_Loadstore_w_slave * tb) {
  Vtb_loadstore_w_slave * core = tb->core;
  core->testcase = T_NO_STALL_LW;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a LW
  //    tick 1. Acknowledge the request with positive reponse data and set the inputs to request a nop
  //    tick 2. Nothing (core latches response data)
  //    tick 3. Nothing (core outputs result of LW)

  //=================================
  //      Tick (0)
  
  tb->reset();

  // LH

  //`````````````````````````````````
  //      Set inputs
  
  core->input_valid_i = 1;

  uint32_t addr = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_lw(addr, reg_addr);

  uint32_t data = rand();
  core->injected_data_i = data;

  //=================================
  //      Tick (1)

  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));         
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_adr_o  ==  addr) &&
                                (core->tb_loadstore_w_slave->wb_we_o   ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_sel_o  ==  0xF)  &&
                                (core->tb_loadstore_w_slave->wb_stb_o  ==  1)    &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));  
  tb->check(COND_register,      (core->reg_write_o               ==  1)    &&
                                (core->reg_addr_o                ==  reg_addr));  
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));         

  //`````````````````````````````````
  //      Set inputs

  tb->_nop();

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  1) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));  
  tb->check(COND_register,      (core->reg_write_o               ==  1)    &&
                                (core->reg_addr_o                ==  reg_addr));  
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));

  //=================================
  //      Tick (3)
  
  tb->tick();

  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  0) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));  
  tb->check(COND_register,      (core->reg_write_o               ==  1)    &&
                                (core->reg_addr_o                ==  reg_addr) &&
                                (core->reg_data_o                ==  data)); 
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  1));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  0) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  0));  
  tb->check(COND_register,      (core->reg_write_o               ==  1)        &&
                                (core->reg_addr_o                ==  reg_addr) &&
                                (core->reg_data_o                ==  data)); 
  tb->check(COND_output_valid,  (core->output_valid_o            ==  1));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_loadstore_w_slave.no_stall.LW_01",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o signal", tb->err_cycles[COND_input_ready]);

  CHECK("tb_loadstore_w_slave.no_stall.LW_02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_loadstore_w_slave.no_stall.LW_03",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_loadstore_w_slave.no_stall.LW_04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_loadstore_w_slave_no_stall_sw(TB_Loadstore_w_slave * tb) {
  Vtb_loadstore_w_slave * core = tb->core;
  core->testcase = T_NO_STALL_SW;

  // The following actions are performed in this test :
  //    tick 0. Set the inputs to request a SW
  //    tick 1. Acknowledge the request and set the inputs to request a nop
  //    tick 2. Nothing (core detects end of request)
  //    tick 3. Nothing (core doesn't output anything)

  //=================================
  //      Tick (0)
   
  tb->reset();

  core->input_valid_i = 1;

  // SW
  
  //`````````````````````````````````
  //      Set inputs

  uint32_t addr = rand();
  uint32_t data = rand();
  uint32_t reg_addr = 1 + rand() % 31;
  tb->_sw(addr, data, reg_addr);

  //=================================
  //      Tick (1)
    
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_adr_o  ==  addr) &&
                                (core->tb_loadstore_w_slave->wb_dat_o  ==  data) &&
                                (core->tb_loadstore_w_slave->wb_we_o   ==  1)    &&
                                (core->tb_loadstore_w_slave->wb_sel_o  ==  0xF)  &&
                                (core->tb_loadstore_w_slave->wb_stb_o  ==  1)    &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));  
  tb->check(COND_register,      (core->reg_write_o               ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));

  //`````````````````````````````````
  //      Set inputs
    
  tb->_nop();

  //=================================
  //      Tick (2)
    
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  1) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));
  tb->check(COND_register,      (core->reg_write_o               ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));

  //=================================
  //      Tick (3)
    
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  0));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0)    &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  0) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  1));
  tb->check(COND_register,      (core->reg_write_o               ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o            ==  0));

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 
  
  tb->check(COND_input_ready,   (core->input_ready_o             ==  1));
  tb->check(COND_wishbone,      (core->tb_loadstore_w_slave->wb_stb_o  ==  0) &&
                                (core->tb_loadstore_w_slave->wb_ack_i  ==  0) &&
                                (core->tb_loadstore_w_slave->wb_cyc_o  ==  0));
  tb->check(COND_register,      (core->reg_write_o               ==  0));
  tb->check(COND_output_valid,  (core->output_valid_o            ==  1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_loadstore_w_slave.no_stall.SW_01",
      tb->conditions[COND_input_ready],
      "Failed to implement the input_ready_o signal", tb->err_cycles[COND_input_ready]);

  CHECK("tb_loadstore_w_slave.no_stall.SW_02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_loadstore_w_slave.no_stall.SW_03",
      tb->conditions[COND_register],
      "Failed to implement the register protocol", tb->err_cycles[COND_register]);

  CHECK("tb_loadstore_w_slave.no_stall.SW_04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Loadstore_w_slave * tb = new TB_Loadstore_w_slave;
  tb->open_trace("waves/loadstore_w_slave.vcd");
  tb->open_testdata("testdata/loadstore_w_slave.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/

  tb_loadstore_w_slave_no_stall_lw(tb);

  tb_loadstore_w_slave_no_stall_sw(tb);

  /************************************************************/

  printf("[LOADSTORE_W_SLAVE]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
