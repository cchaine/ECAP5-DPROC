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

#include "Vecap5_dproc_ecap5_dproc_pkg.h"
#include "Vecap5_dproc.h"
#include "testbench.h"

#define KO * 1024
#define MAX_BINARY_SIZE (32 KO)

#define MAX_TICKCOUNT 3000

#define END_ADDRESS 0xFFEEBBCC

class TB_Riscv_tests: public Testbench<Vecap5_dproc> {
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
    
    // Clear memory
    memset(memory, 0, MAX_BINARY_SIZE);
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

  void get_register(uint8_t addr, uint32_t * value) {
    const svScope scope = svGetScopeFromName("TOP.ecap5_dproc.regs_inst");
    assert(scope);
    svSetScope(scope);
    this->core->get_register_value((svLogicVecVal*)&addr, (svLogicVecVal*)value);
  }

  void close_trace() {
    if(this->trace != NULL) {
      this->trace->close();
      this->trace = NULL;
    }
  }
};

void tb_riscv_tests_simple(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-simple.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-simple.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  CHECK("riscv-tests.simple.01",
      tb->is_done,
      "Failed to terminate (timeout)");
  CHECK("riscv-tests.simple.02",
      true,
      "Failed");

  tb->close_trace();
}

void tb_riscv_tests_add(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-add.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-add.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.add.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.add.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_addi(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-addi.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-addi.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.addi.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.addi.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_and(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-and.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-and.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.and.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.and.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_andi(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-andi.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-andi.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.andi.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.andi.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_auipc(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-auipc.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-auipc.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.auipc.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.auipc.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_beq(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-beq.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-beq.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.beq.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.beq.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_bge(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-bge.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-bge.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.bge.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.bge.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_bgeu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-bgeu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-bgeu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.bgeu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.bgeu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_blt(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-blt.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-blt.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.blt.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.blt.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_bltu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-bltu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-bltu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.bltu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.bltu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_bne(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-bne.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-bne.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.bne.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.bne.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_fence_i(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-fence_i.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-fence_i.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.fence_i.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.fence_i.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_jal(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-jal.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-jal.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.jal.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.jal.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_jalr(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-jalr.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-jalr.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.jalr.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.jalr.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lb(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lb.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lb.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lb.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lb.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lbu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lbu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lbu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lbu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lbu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lh(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lh.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lh.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lh.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lh.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lhu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lhu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lhu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lhu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lhu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lw(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lw.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lw.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lw.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lw.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_lui(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-lui.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-lui.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.lui.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.lui.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_ma_data(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-ma_data.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-ma_data.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.ma_data.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.ma_data.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_or(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-or.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-or.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.or.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.or.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_ori(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-ori.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-ori.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.ori.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.ori.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sb(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sb.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sb.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sb.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sb.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sh(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sh.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sh.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sh.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sh.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sw(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sw.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sw.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sw.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sw.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sll(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sll.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sll.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sll.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sll.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_slli(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-slli.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-slli.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.slli.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.slli.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_slt(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-slt.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-slt.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.slt.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.slt.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_slti(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-slti.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-slti.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.slti.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.slti.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sltiu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sltiu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sltiu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sltiu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sltiu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sltu(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sltu.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sltu.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sltu.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sltu.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sra(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sra.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sra.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sra.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sra.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_srai(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-srai.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-srai.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.srai.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.srai.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_srl(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-srl.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-srl.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.srl.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.srl.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_srli(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-srli.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-srli.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.srli.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.srli.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_sub(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-sub.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-sub.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.sub.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.sub.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_xor(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-xor.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-xor.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.xor.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.xor.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

void tb_riscv_tests_xori(TB_Riscv_tests * tb) {
  tb->open_trace("waves/riscv-tests-xori.vcd");

  Vecap5_dproc * core = tb->core;
  tb->reset();  

  tb->set_memory("riscv-tests/tests/rv32ui-p-xori.elf");

  while(!tb->is_done && tb->tickcount < MAX_TICKCOUNT) {
    tb->tick();
  }

  uint32_t testcase;
  tb->get_register(3, &testcase);
  uint32_t result;
  tb->get_register(4, &result);

  CHECK("riscv-tests.xori.01",
      tb->is_done,
      "Failed to terminate (timeout)");

  CHECK("riscv-tests.xori.02",
      result == 1,
      "Failed during testcase", testcase);

  tb->close_trace();
}

int main(int argc, char ** argv, char ** env) {
  srand(time(NULL));
  Verilated::traceEverOn(true);

  // Check arguments
  bool verbose = parse_verbose(argc, argv);

  TB_Riscv_tests * tb = new TB_Riscv_tests();
  tb->open_testdata("testdata/riscv-tests.csv");
  tb->set_debug_log(verbose);

  /************************************************************/

  tb_riscv_tests_simple(tb);
  tb_riscv_tests_add(tb);
  tb_riscv_tests_addi(tb);
  tb_riscv_tests_and(tb);
  tb_riscv_tests_andi(tb);
  tb_riscv_tests_auipc(tb);
  tb_riscv_tests_beq(tb);
  tb_riscv_tests_bge(tb);
  tb_riscv_tests_bgeu(tb);
  tb_riscv_tests_blt(tb);
  tb_riscv_tests_bltu(tb);
  tb_riscv_tests_bne(tb);
  // tb_riscv_tests_fence_i(tb);
  tb_riscv_tests_jal(tb);
  tb_riscv_tests_jalr(tb);
  tb_riscv_tests_lb(tb);
  tb_riscv_tests_lbu(tb);
  tb_riscv_tests_lh(tb);
  tb_riscv_tests_lhu(tb);
  tb_riscv_tests_lw(tb);
  tb_riscv_tests_lui(tb);
  // tb_riscv_tests_ma_data(tb);
  tb_riscv_tests_or(tb);
  tb_riscv_tests_ori(tb);
  tb_riscv_tests_sb(tb);
  tb_riscv_tests_sh(tb);
  tb_riscv_tests_sw(tb);
  tb_riscv_tests_sll(tb);
  tb_riscv_tests_slli(tb);
  tb_riscv_tests_slt(tb);
  tb_riscv_tests_slti(tb);
  tb_riscv_tests_sltiu(tb);
  tb_riscv_tests_sltu(tb);
  tb_riscv_tests_sra(tb);
  tb_riscv_tests_srai(tb);
  tb_riscv_tests_srl(tb);
  tb_riscv_tests_srli(tb);
  tb_riscv_tests_sub(tb);
  tb_riscv_tests_xor(tb);
  tb_riscv_tests_xori(tb);

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
