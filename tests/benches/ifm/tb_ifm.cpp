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

    Testbench<Vtb_ifm>::reset();
  }
};

enum CondId {
  COND_state,
  COND_wishbone,
  COND_output,
  COND_output_valid,
  __CondIdEnd
};

void tb_ifm_no_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 1;

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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 0;

  core->output_ready_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address) &&
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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  3));         
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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  Vtb_ifm_ecap5_dproc_pkg::boot_address));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address + 4) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.no_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_ifm.no_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.no_stall.03",
      tb->conditions[COND_output],
      "Failed to implement the output signals", tb->err_cycles[COND_output]);

  CHECK("tb_ifm.no_stall.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 2;

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
  core->drq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 1;

  core->output_ready_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  4));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  4));
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_stall_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  

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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  3));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.memory_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_ifm.memory_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_ifm_memory_wait_state(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 3;

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
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));         
  
  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  2));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  2));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  

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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  3));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  1));  

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  0));         
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.memory_wait_state.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_ifm.memory_wait_state.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_ifm_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 4;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for pipeline stall, no interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core holds response)
  //    tick 4. Unstall pipeline (core holds response)
  //    tick 5. Nothing (core outputs request)
  //    tick 6. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;

  core->wb_stall_i = 0;

  // Stall the pipeline
  core->output_ready_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));

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

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  3));

  //`````````````````````````````````
  //      Set inputs
  
  core->wb_dat_i = 0;
  core->wb_ack_i = 0;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  5));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  Vtb_ifm_ecap5_dproc_pkg::boot_address));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  5));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  Vtb_ifm_ecap5_dproc_pkg::boot_address));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //`````````````````````````````````
  //      Set inputs
  
  core->output_ready_i = 1;

  //=================================
  //      Tick (5)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  0));
  tb->check(COND_wishbone,      (core->wb_stb_o              ==  0)    &&
                                (core->wb_cyc_o              ==  0));  
  tb->check(COND_output,        (core->instr_o               ==  data) &&
                                (core->pc_o                  ==  Vtb_ifm_ecap5_dproc_pkg::boot_address));
  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         

  //=================================
  //      Tick (6)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_state,         (core->tb_ifm->dut->state_q  ==  1));         
  tb->check(COND_wishbone,      (core->wb_adr_o              ==  Vtb_ifm_ecap5_dproc_pkg::boot_address + 4) &&
                                (core->wb_we_o               ==  0)    &&
                                (core->wb_sel_o              ==  0xF)  &&
                                (core->wb_stb_o              ==  1)    &&
                                (core->wb_cyc_o              ==  1));  
  tb->check(COND_output_valid,  (core->output_valid_o        ==  0));         
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.pipeline_stall.01",
      tb->conditions[COND_state],
      "Failed to implement the state machine", tb->err_cycles[COND_state]);

  CHECK("tb_ifm.pipeline_stall.02",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.pipeline_stall.03",
      tb->conditions[COND_output],
      "Failed to implement the output signals", tb->err_cycles[COND_output]);

  CHECK("tb_ifm.pipeline_stall.04",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

/*============================================*/
/*                   Debug                    */
/*============================================*/

void tb_ifm_debug_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 5;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall with debug request
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->drq_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  core->drq_i = 0;

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
  //      Checks 

  tb->check(COND_output_valid, (core->output_valid_o == 0));
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_during_request.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
  
  CHECK("tb_ifm.debug_during_request.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 6;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and debug request (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs

  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->drq_i = 1;

  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->drq_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_during_ack.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_during_ack.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 7;

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
  core->drq_i = 0;
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
  
  core->drq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_during_wait.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_during_wait.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 8;

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
  core->drq_i = 0;
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
  
  core->drq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_during_memory_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_during_memory_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 9;

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
  core->drq_i = 0;
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

  core->drq_i = 1;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 0;

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks
  
  CHECK("tb_ifm.debug_on_output_handshake.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_on_output_handshake.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 10;

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
  core->drq_i = 0;
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
  
  core->drq_i = 1;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_during_pipeline_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_during_pipeline_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_debug_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 11;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and debug request (core makes request)
  //    tick 2. Debug request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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
  core->drq_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 1;

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
  
  core->drq_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.debug_back_to_back.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.debug_back_to_back.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

/*============================================*/
/*                 Interrupt                  */
/*============================================*/

void tb_ifm_interrupt_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 12;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall with interrupt request
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->irq_i = 1;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  core->irq_i = 0;

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
  //      Checks 

  tb->check(COND_output_valid, (core->output_valid_o == 0));
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_during_request.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
  
  CHECK("tb_ifm.interrupt_during_request.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 13;

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
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_during_ack.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_during_ack.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 14;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Nothing (core waits response)
  //    tick 2. Interrupt request (core waits response)
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
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_during_wait.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_during_wait.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 15;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for memory stall not interrupt
  //    tick 1. Nothing (core holds request)
  //    tick 2. Interrupt request (core holds request)
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
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_during_memory_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_during_memory_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 16;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Interrupt request (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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

  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks
  
  CHECK("tb_ifm.interrupt_on_output_handshake.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_on_output_handshake.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 17;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for pipeline stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Interrupt request (core holds response)
  //    tick 4. Nothing (core holds response)
  //    tick 5. Nothing (core cancels response)
  //    tick 6. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_during_pipeline_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_during_pipeline_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_interrupt_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 18;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and interrupt request (core makes request)
  //    tick 2. Interrupt request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.interrupt_back_to_back.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.interrupt_back_to_back.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

/*============================================*/
/*                   Branch                   */
/*============================================*/

void tb_ifm_branch_during_request(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 19;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall with branch request
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  // Leave the reset state
  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address));
  
  //`````````````````````````````````
  //      Set inputs
  
  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  core->branch_i = 0;

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
  //      Checks 

  tb->check(COND_output_valid, (core->output_valid_o == 0));
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));

  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_during_request.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
  
  CHECK("tb_ifm.branch_during_request.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_during_ack(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 20;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and branch request (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs

  core->irq_i = 0;
  core->drq_i = 0;
  core->branch_i = 0;
  core->output_ready_i = 1;
  core->wb_stall_i = 0;

  //=================================
  //      Tick (1)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

  uint32_t data = rand();
  core->wb_dat_i = data;
  core->wb_ack_i = 1;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs

  core->branch_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_during_ack.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_during_ack.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_during_wait(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 21;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Nothing (core waits response)
  //    tick 2. Branch request (core waits response)
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
  core->drq_i = 0;
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
  
  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_during_wait.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_during_wait.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_during_memory_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 22;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for memory stall not interrupt
  //    tick 1. Nothing (core holds request)
  //    tick 2. Branch request (core holds request)
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
  core->drq_i = 0;
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
  
  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_during_memory_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_during_memory_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_on_output_handshake(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 23;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Branch request (core latches response)
  //    tick 3. Nothing (core outputs response)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_output_valid,  (core->output_valid_o        ==  1));         
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks
  
  CHECK("tb_ifm.branch_on_output_handshake.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_on_output_handshake.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_during_pipeline_stall(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 24;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for pipeline stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Nothing (core latches response)
  //    tick 3. Branch request (core holds response)
  //    tick 4. Nothing (core holds response)
  //    tick 5. Nothing (core cancels response)
  //    tick 6. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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
  
  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_during_pipeline_stall.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_during_pipeline_stall.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

void tb_ifm_branch_back_to_back(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 25;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data and interrupt request (core makes request)
  //    tick 2. Branch request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->irq_i = 0;
  core->drq_i = 0;
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

  core->branch_i = 1;
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

  //=================================
  //      Tick (2)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 1;
  core->boffset_i = rand();

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
  
  core->branch_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.branch_back_to_back.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);

  CHECK("tb_ifm.branch_back_to_back.02",
      tb->conditions[COND_output_valid],
      "Failed to implement the output_valid_o signal", tb->err_cycles[COND_output_valid]);
}

/*============================================*/
/*                 Precedence                 */
/*============================================*/

void tb_ifm_precedence_debug(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 26;

  // The following actions are performed in this test :
  //    tick 0. Set inputs for no stall not interrupt
  //    tick 1. Acknowledge request with response data (core makes request)
  //    tick 2. Debug, Interrupt and Branch request (core latches response)
  //    tick 3. Nothing (core cancels output)
  //    tick 4. Nothing (core makes request)
  
  //=================================
  //      Tick (0)
  
  tb->reset();

  //`````````````````````````````````
  //      Set inputs
  
  core->drq_i = 0;
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

  core->drq_i = 1;
  core->irq_i = 1;
  core->branch_i = 1;
  core->boffset_i = rand() % 0x7FFFFFFF;

  //=================================
  //      Tick (3)
  
  tb->tick();

  //`````````````````````````````````
  //      Set inputs
  
  core->branch_i = 0;
  core->irq_i = 0;
  core->drq_i = 0;
  
  //=================================
  //      Tick (4)
  
  tb->tick();

  //`````````````````````````````````
  //      Checks 


  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::debug_address));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_ifm.precedence_debug.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_ifm_precedence_interrupt(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 27;

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
  
  core->drq_i = 0;
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
  core->boffset_i = rand() % 0x7FFFFFFF;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::interrupt_address));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_ifm.precedence_interrupt.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_ifm_precedence_branch(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 28;

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
  
  core->drq_i = 0;
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
  uint32_t boffset = rand() % 0xFFFFF;
  core->boffset_i = boffset;

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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + boffset));

  //`````````````````````````````````
  //      Formal Checks 

  CHECK("tb_ifm.precedence_branch.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

void tb_ifm_precedence_increment(TB_Ifm * tb) {
  Vtb_ifm * core = tb->core;
  core->testcase = 29;

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
  
  core->drq_i = 0;
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

  tb->check(COND_wishbone, (core->wb_adr_o == Vtb_ifm_ecap5_dproc_pkg::boot_address + 4));
  
  //`````````````````````````````````
  //      Formal Checks 
  
  CHECK("tb_ifm.precedence_increment.01",
      tb->conditions[COND_wishbone],
      "Failed to implement the wishbone protocol", tb->err_cycles[COND_wishbone]);
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  bool verbose = parse_verbose(argc, argv);

  TB_Ifm * tb = new TB_Ifm;
  tb->open_trace("waves/ifm.vcd");
  tb->open_testdata("testdata/ifm.csv");
  tb->set_debug_log(verbose);
  tb->init_conditions(__CondIdEnd);

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
