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

#include "Vecap5_dproc.h"
#include "testbench.h"
#include "elf.h"

#define KO * 1024
#define MAX_BINARY_SIZE (32 KO)

#define MAX_TICKCOUNT 3000

#define OUTPUT_ADDRESS 0x80000000
#define END_ADDRESS 0xA0000000

class TB_Emulator: public Testbench<Vecap5_dproc> {
public:
  uint8_t memory[MAX_BINARY_SIZE];
  bool is_done;

  void reset() {
    this->is_done = 0;
    this->tickcount = 0;

    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vecap5_dproc>::reset();
  }

  void tick() {
    static uint8_t state = 0;

    // handle wishbone bus
    switch(state) {
      case 0: {
        if((this->core->wb_stb_o == 1) && (this->core->wb_cyc_o == 1)) {
          uint32_t data = 0;
          // check test end
          if(this->core->wb_adr_o == END_ADDRESS) {
            this->is_done = 1;
          } else if(this->core->wb_adr_o == OUTPUT_ADDRESS && this->core->wb_we_o == 1) {
            char c = this->core->wb_dat_o & 0xFF;
            printf("%c", c);
          // check overflow
          } else if(this->core->wb_adr_o >= MAX_BINARY_SIZE) {
            printf("Runtime memory overflow\n  Requested address : %08x, Memory end address : %08x\n\n", this->core->wb_adr_o, MAX_BINARY_SIZE-1);
          } else {
            if(this->core->wb_we_o == 0) {
              // Read
              memcpy(&data, memory + this->core->wb_adr_o, 4);
              switch(this->core->wb_sel_o) {
                case 0x1:
                  data &= 0xFF;
                  break;
                case 0x3:
                  data &= 0xFFFF;
                  break;
                case 0xF:
                  break;
                default:
                  printf("Invalid wishbone sel signal during read: %08x\n", this->core->wb_sel_o);
                  break;
              }
            } else {
              // Write
              uint8_t size = 0;
              switch(this->core->wb_sel_o) {
                case 0x1:
                  size = 1;
                  break;
                case 0x3:
                  size = 2;
                  break;
                case 0xF:
                  size = 4;
                  break;
                default:
                  printf("Invalid wishbone sel signal during write: %08x\n", this->core->wb_sel_o);
                  break;
              }
              memcpy(memory + this->core->wb_adr_o, &this->core->wb_dat_o, size);
            }
          }
          this->core->wb_dat_i = data;
          this->core->wb_ack_i = 1;
          state = 1;
          break;
        }
      }
      case 1: {
        this->core->wb_dat_i = 0;
        this->core->wb_ack_i = 0;
        state = 0;
        break;
      }
    }

    Testbench<Vecap5_dproc>::tick();
  }

  void set_memory(std::string path) {
    memset(memory, 0, MAX_BINARY_SIZE);
    load_elf(path, memory, MAX_BINARY_SIZE);
  }
};

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  TB_Emulator * tb = new TB_Emulator();

  tb->reset();  

  uint32_t max_tickcount = MAX_TICKCOUNT;
  if(argc >= 2) {
    tb->set_memory(argv[1]);
    if(argc >= 3) {
      tb->open_trace(argv[2]);
      if(argc == 4) {
        max_tickcount = atoi(argv[3]); 
      }
    }
  } else {
    printf("Usage: %s elf_binary vcd_output max_tickcount\n", argv[0]);
    return -1;
  }

  while(!tb->is_done && tb->tickcount < max_tickcount) {
    tb->tick();
  }

  tb->close_trace();

  if(tb->tickcount >= max_tickcount) {
    printf("\nKilled: Timeout\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
