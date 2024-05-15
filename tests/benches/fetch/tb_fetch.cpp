/*           __        _
 *  ________/ /  ___ _(_)__  ___
 * / __/ __/ _ \/ _ `/ / _ \/ -_)
 * \__/\__/_//_/\_,_/_/_//_/\__/
 * 
 * Copyright (C) Clément Chaine
 * This file is part of ECAP5-DPROC <https://github.com/ecap5/ECAP5-DPROC>
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

#include "Vtb_fetch.h"
#include "testbench.h"
#include "Vtb_fetch_ecap5_dproc_pkg.h"
#include "Vtb_fetch_fetch.h"
#include "Vtb_fetch_tb_fetch.h"

enum CondId {
  COND_state,
  COND_wishbone,
  COND_output,
  COND_output_valid,
  __CondIdEnd
};

enum TestcaseId {
  T_NO_STALL                         =  1,
  T_MEMORY_STALL                     =  2,
  T_MEMORY_WAIT                      =  3,
  T_PIPELINE_WAIT                   =  4,
  T_JUMP_DURING_REQUEST             =  5,
  T_JUMP_DURING_ACK                 =  6,
  T_JUMP_DURING_WAIT                =  7,
  T_JUMP_DURING_MEMORY_STALL        =  8,
  T_JUMP_ON_OUTPUT_HANDSHAKE        =  9,
  T_JUMP_DURING_PIPELINE_WAIT      =  10,
  T_JUMP_BACK_TO_BACK               =  11,
  T_PRECEDENCE_INTERRUPT             =  12,
  T_PRECEDENCE_BRANCH                =  13,
  T_PRECEDENCE_INCREMENT             =  14,
  T_RESET                            =  15
};

class TB_Fetch : public Testbench<Vtb_fetch> {
public:
  void reset() {
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vtb_fetch>::reset();
  }
};

void tb_fetch_reset(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_RESET;

  tb->reset();

  CHECK("tb_fetch.reset.01",
      false,
      "TODO");
}

void tb_fetch_no_stall(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_NO_STALL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 0;

  core->output_ready_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  3));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  core->tb_fetch->dut->BOOT_ADDRESS));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS + 4) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.no_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_fetch.no_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.no_stall.03",
      tb->conditions[COND_output],
      "Failed to implement the output signals", tb->err_cycles[COND_output]);

  CHECK("tb_fetch.no_stall.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_memory_stall(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_MEMORY_STALL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no memory stall, no interrupt
  //    tick 1. Nothing (core holds request)
  //    tick 2. Unstall memory (core holds request)
  //    tick 3. Acknowledge request with response data (core makes response)
  //    tick 4. Nothing (core latches response)
  //    tick 5. Nothing (core outputs response)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 1;

  core->output_ready_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  4));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  4));
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  2));
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  3));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output_valid,  (core->output_valid_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.memory_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_fetch.memory_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.memory_stall.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_memory_wait(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_MEMORY_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall, no interrupt
  //    tick 1. Nothing (core makes request)
  //    tick 2. Nothing (core waits for response)
  //    tick 3. Acknowledge request with response data (core waits for response)
  //    tick 4. Nothing (core latches response)
  //    tick 5. Nothing (core outputs response)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  1));         
  tb->check(COND_output_valid,  (core->output_valid_o == 0));
  
  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  2));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  2));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  3));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o == 0));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output_valid,  (core->output_valid_o == 1));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.memory_wait.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_fetch.memory_wait.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.memory_wait.03",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_pipeline_wait(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_PIPELINE_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for pipeline stall, no interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core holds response)
  //    tick 4. Unstall pipeline (core holds response)
  //    tick 5. Nothing (core prepares new request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 0;

  // Stall the pipeline
  core->output_ready_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  1));

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  3));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  5));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  core->tb_fetch->dut->BOOT_ADDRESS));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  5));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  core->tb_fetch->dut->BOOT_ADDRESS));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_fetch->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  core->tb_fetch->dut->BOOT_ADDRESS + 4) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.pipeline_wait.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_fetch.pipeline_wait.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.pipeline_wait.03",
      tb->conditions[COND_output],
      "Failed to implement the output signals", tb->err_cycles[COND_output]);

  CHECK("tb_fetch.pipeline_wait.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_after_reset(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_DURING_REQUEST;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall with interrupt request
  //    tick 1. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->irq_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_after_reset.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
  
  CHECK("tb_fetch.jump_after_reset.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_during_ack(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_DURING_ACK;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and interrupt request (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs

  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->irq_i = 1;

  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->irq_i = 0;

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o  ==  0));         
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_during_ack.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_during_ack.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_during_wait(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_DURING_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Nothing (core waits response)
  //    tick 2. Debug request (core waits response)
  //    tick 3. Nothing (core waits response)
  //    tick 4. Acknowledge request with response data (core waits response)
  //    tick 5. Nothing (core latches response)
  //    tick 6. Nothing (core output cancelled)
  //    tick 7. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //=================================
  //      Tick (7)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_during_wait.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_during_wait.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_during_memory_stall(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_DURING_MEMORY_STALL;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for memory stall not interrupt
  //    tick 1. Nothing (core holds request)
  //    tick 2. Debug request (core holds request)
  //    tick 3. Nothing (core holds request)
  //    tick 4. Unstall memory (core holds request)
  //    tick 5. Acknowledge request with response data (core latches response)
  //    tick 6. Nothing (core latches response)
  //    tick 7. Nothing (core output cancelled)
  //    tick 8. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 1;
  
  //=================================
  //      Tick (1)
  
  tb->tick();
  
  //=================================
  //      Tick (2)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (6)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;
  
  //=================================
  //      Tick (7)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //=================================
  //      Tick (8)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_during_memory_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_during_memory_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_on_output_handshake(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_ON_OUTPUT_HANDSHAKE;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Debug request (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->irq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks
  
  CHECK("tb_fetch.jump_on_output_handshake.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_on_output_handshake.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_during_pipeline_wait(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_DURING_PIPELINE_WAIT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for pipeline stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Debug request (core holds response)
  //    tick 4. Nothing (core holds response)
  //    tick 5. Nothing (core cancels response)
  //    tick 6. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 0;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //=================================
  //      Tick (6)
  
  tb->tick();
  
  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_during_pipeline_wait.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_during_pipeline_wait.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_jump_back_to_back(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_JUMP_BACK_TO_BACK;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and interrupt request (core makes request)
  //    tick 2. Debug request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;
  core->irq_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 1;

  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.jump_back_to_back.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_fetch.jump_back_to_back.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_fetch_precedence_interrupt(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_PRECEDENCE_INTERRUPT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Interrupt and Branch request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->irq_i = 1;
  core->branch_i = 1;
  core->branch_target_i = rand() % 0x7FFFFFFF;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;
  core->irq_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->INTERRUPT_ADDRESS));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_fetch.precedence_interrupt.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_fetch_precedence_branch(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_PRECEDENCE_BRANCH;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Branch request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  core->branch_i = 1;
  uint32_t branch_target = rand() % 0xFFFFF;
  core->branch_target_i = branch_target;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == branch_target));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_fetch.precedence_branch.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_fetch_precedence_increment(TB_Fetch * tb) {
  Vtb_fetch * core = tb->core;
  core->testcase = T_PRECEDENCE_INCREMENT;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->irq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == core->tb_fetch->dut->BOOT_ADDRESS + 4));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_fetch.precedence_increment.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Fetch * tb = new TB_Fetch;
  tb->open_trace("waves/fetch.vcd");
  tb->open_testdata("testdata/fetch.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

  /************************************************************/
  
  tb_fetch_reset(tb);

  tb_fetch_no_stall(tb);

  tb_fetch_memory_stall(tb);

  tb_fetch_memory_wait(tb);

  tb_fetch_pipeline_wait(tb);

  tb_fetch_jump_after_reset(tb);
  tb_fetch_jump_during_ack(tb);
  tb_fetch_jump_during_wait(tb);
  tb_fetch_jump_during_memory_stall(tb);
  tb_fetch_jump_on_output_handshake(tb);
  tb_fetch_jump_during_pipeline_wait(tb);
  tb_fetch_jump_back_to_back(tb);

  tb_fetch_precedence_interrupt(tb);
  tb_fetch_precedence_branch(tb);
  tb_fetch_precedence_increment(tb);

  /************************************************************/

  printf("[FETCH]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
