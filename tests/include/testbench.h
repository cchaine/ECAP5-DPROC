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

#ifndef TESTBENCH_H
#define TESTBENCH_H

// This macro is used to allow easier parsing of all the checks
#define CHECK(TB, COND, MSG) tb->check(TB, COND, MSG)

template<class Module> class Testbench {
public:
  unsigned long tickcount;
  Module * core;
  VerilatedVcdC * trace;
  bool success;
  const char * testdata_path;
  bool debug_log;

  Testbench() {
    this->tickcount = 0;
    this->core = new Module;
    this->success = true;
    this->testdata_path = NULL;
    this->debug_log = false;
  }

  ~Testbench() {
    this->core->final();
    this->trace->close();
    this->trace = NULL;
    delete this->core;
    this->core = NULL;
  }

  void open_trace(const char * path) {
    if(this->trace == NULL) {
      this->trace = new VerilatedVcdC;
      this->core->trace(this->trace, 99);
      this->trace->open(path);
    }
  }

  void set_debug_log(bool debug_log) {
    this->debug_log = debug_log;
  }

  void tick() {
    this->tickcount += 1;

    core->clk_i = 0;
    core->eval();
    if(this->trace) {
      this->trace->dump(10 * this->tickcount - 2);
    }

    core->clk_i = 1;
    core->eval();
    if(this->trace) {
      this->trace->dump(10 * this->tickcount);
    }

    core->clk_i = 0;
    core->eval();
    if(this->trace) {
      this->trace->dump(10 * this->tickcount+5);
      this->trace->flush();
    }
  }

  void open_testdata(const char * path) {
    this->testdata_path = path;
    // open the file to clear the content
    FILE * f = fopen(this->testdata_path, "w");
    assert(f);
    fclose(f);
  }

  void check(const char * testbench, bool condition, const char * msg) {
    if(this->testdata_path != NULL) {
      FILE * f = fopen(this->testdata_path, "a");
      if(condition) {
        fprintf(f, "%s;%d\n", testbench, true);
      } else {
        fprintf(f, "%s;%d;%s\n", testbench, false, msg);
      }
      fclose(f);
    }
    if(this->debug_log) {
      printf("[%s]: ", testbench);
      if(condition) {
        printf("OK\n");
      } else {
        printf("%s\n", msg);
      }
    }

    if(!condition) {
        this->success = false;
    }
  }
};

bool parse_verbose(int argc, char ** argv) {
  return (argc == 2) && (strncmp(argv[1], "-v", 2) == 0);
}

#endif
