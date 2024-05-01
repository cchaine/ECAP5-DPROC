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

#include "Vecap5_dproc_ecap5_dproc_pkg.h"
#include "Vecap5_dproc.h"
#include "testbench.h"

#define KO * 1024
#define MAX_BINARY_SIZE (32 KO)

#define MAX_TICKCOUNT 1000

class TB_Riscv_tests: public Testbench<Vecap5_dproc> {
public:
  uint8_t memory[MAX_BINARY_SIZE];

  void reset() {
    this->core->rst_i = 1;
    for(int i = 0; i < 5; i++) {
      this->tick();
    }
    this->core->rst_i = 0;

    Testbench<Vecap5_dproc>::reset();
    
    // Clear memory
    memset(memory, 0, MAX_BINARY_SIZE);
  }

  void tick() {
    static uint8_t state = 0;

    // handle wishbone bus
    switch(state) {
      case 0: {
        if((this->core->wb_stb_o == 1) && (this->core->wb_cyc_o == 1)) {
          // check overflow
          uint32_t data;
          if(this->core->wb_adr_o >= MAX_BINARY_SIZE) {
            printf("Runtime memory overflow\n  Requested address : %08x, Memory end address : %08x\n\n", this->core->wb_adr_o, MAX_BINARY_SIZE-1);
            data = 0;
          } else {
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
                printf("Invalid wishbone sel signal : %08x\n", this->core->wb_sel_o);
                break;
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
    this->load_elf(path, memory);
  }

  void dump_memory(uint32_t size) {
    if(size > MAX_BINARY_SIZE) {
      size = MAX_BINARY_SIZE;
    }

    char ascii[17];
    int i, j;
    ascii[16] = '\0';
    printf("%08x: ", 0);
    for (i = 0; i < size; ++i) {
      printf("%02X", ((unsigned char*)this->memory)[i]);
      if(i % 2 == 1) {
        printf(" ");
      }
      if (((unsigned char*)this->memory)[i] >= ' ' && ((unsigned char*)this->memory)[i] <= '~') {
        ascii[i % 16] = ((unsigned char*)this->memory)[i];
      } else {
        ascii[i % 16] = '.';
      }
      if ((i+1) % 8 == 0 || i+1 == size) {
        if ((i+1) % 16 == 0) {
          printf(" %s \n", ascii);
          if(i+1 < size) {
            printf("%08x: ", i);
          }
        } else if (i+1 == size) {
          ascii[(i+1) % 16] = '\0';
          if ((i+1) % 16 <= 8) {
            printf(" ");
          }
          for (j = (i+1) % 16; j < 16; ++j) {
            printf("   ");
          }
          printf(" %s \n", ascii);
        }
      }
    }
  }

  void load_elf(std::string path, void * buffer) {
    FILE * fd = fopen(path.c_str(), "rb");
    if(fd == NULL) {
      printf("Couldn't find elf file at path %s\n", path.c_str());
      return;
    }

    // Check elf magic
    uint8_t magic = 0;
    fread(&magic, 1, 1, fd);
    if(magic != 0x7F) {
      printf("The file %s is not an elf binary\n", path.c_str());
      return;
    }

    // Read elf header and find elf program header
    uint32_t ph_offset = 0;
    fseek(fd, 28, SEEK_SET);
    fread(&ph_offset, 4, 1, fd);

    // Find the number of program segments
    uint16_t ph_entry_size = 0;
    uint16_t ph_num_entries = 0;
    fseek(fd, 42, SEEK_SET);
    fread(&ph_entry_size, 2, 1, fd);
    fseek(fd, 44, SEEK_SET);
    fread(&ph_num_entries, 2, 1, fd);

    // For each segment, copy the data to the provided buffer
    for(int i = 0; i < ph_num_entries; i++) {
      uint32_t p_type = 0;
      fseek(fd, ph_offset + ph_entry_size * i, SEEK_SET);
      fread(&p_type, 4, 1, fd);

      uint32_t p_offset = 0;
      fseek(fd, ph_offset + ph_entry_size * i + 4, SEEK_SET);
      fread(&p_offset, 4, 1, fd);

      uint32_t p_vaddr = 0; 
      fseek(fd, ph_offset + ph_entry_size * i + 8, SEEK_SET);
      fread(&p_vaddr, 4, 1, fd);

      uint32_t p_filesz = 0;
      fseek(fd, ph_offset + ph_entry_size * i + 16, SEEK_SET);
      fread(&p_filesz, 4, 1, fd);

      uint32_t p_memsz = 0;
      fseek(fd, ph_offset + ph_entry_size * i + 20, SEEK_SET);
      fread(&p_memsz, 4, 1, fd);
      
      // If it is a LOAD segment
      if(p_type == 1) {
        // Check if bigger than the memory size
        if(p_vaddr + p_memsz > MAX_BINARY_SIZE) {
          printf("Memory overflow with segment %d when loading binary %s\n  Memory end address : %08x, Segment end address: %08x\n\n", i, path.c_str(), MAX_BINARY_SIZE-1, p_vaddr + p_memsz - 1);
          return;
        }

        // Copy the segment data
        fseek(fd, p_offset, SEEK_SET);
        fread(memory + p_vaddr, 1, p_filesz, fd);
      }
    } 

    fclose(fd);
  }
};

void tb_riscv_tests_simple(TB_Riscv_tests * tb) {
  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/rv32ui-p-simple");

  while(tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Riscv_tests * tb = new TB_Riscv_tests();
  tb->open_trace("waves/riscv-tests.vcd");
  tb->open_testdata("testdata/riscv-tests.csv");
  tb->set_debug_log(verbose);

  /************************************************************/

  tb_riscv_tests_simple(tb);

  /************************************************************/

  printf("[RISCV-TESTS]: ");
  if(tb->success) {
    printf("Done\n");
  } else {
    printf("Failed\n");
  }

  delete tb;
  exit(EXIT_SUCCESS);
}
