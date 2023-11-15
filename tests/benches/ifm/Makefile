#           __        _
#  ________/ /  ___ _(_)__  ___
# / __/ __/ _ \/ _ `/ / _ \/ -_)
# \__/\__/_//_/\_,_/_/_//_/\__/
# 
# Copyright (C) Clément Chaine
# This file is part of ECAP5-DPROC <https://github.com/cchaine/ECAP5-DPROC>
# 
# ECAP5-DPROC is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# ECAP5-DPROC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with ECAP5-DPROC.  If not, see <http://www.gnu.org/licenses/>.

#
# ================ Arguments ================
# 
# PROJECT_ROOT: path to the root directory of the project
# BUILD_DIR : path to the build directory
# INC_DIR : path to the hdl sources
# TEST_INC_DIR:  path to the directory containing the C headers used in tests
#

BENCHES = tb_ifm
BENCH_BINARIES = $(addprefix $(BUILD_DIR)/, $(BENCHES))

CFLAGS = -Wall -Og -g -I$(VERILATOR_ROOT)/include -I$(VERILATOR_ROOT)/include/vltstd

INCLUDES = $(shell find $(INC_DIR) -name '*.svh')

TEST_LIBRARIES = 
TEST_LIBRARIES_BINARIES = $(addprefix $(BUILD_LIBS_DIR)/, $(TEST_LIBRARIES))

all: $(BENCH_BINARIES)

#
# ================ Verilation process for testbenches ================
# 
$(BUILD_TEST_DIR)/V%__ALL.a: $(BUILD_SRC_DIR)/Vifm__ALL.a $(BUILD_TEST_DIR)/V%.cpp $(BUILD_TEST_DIR)/V%.h $(BUILD_TEST_DIR)/V%.mk
	make -C $(BUILD_TEST_DIR) -f V$*.mk
$(BUILD_TEST_DIR)/V%.mk: %.sv
	verilator $(VERILATOR_OPTS) $(VERILATOR_WARNINGS) -I$(PROJECT_ROOT)/src -I$(TEST_LIBS_DIR) --Mdir $(BUILD_TEST_DIR) --prefix V$* -cc $(INCLUDES) $*.sv
$(BUILD_TEST_DIR)/V%.cpp $(BUILD_TEST_DIR)/V%.h: $(BUILD_TEST_DIR)/V%.mk ;

#
# ================ Compile tests ================
# 
TEST_LIBRARIES_LINK_ARGS = $(addprefix "-l:", $(TEST_LIBRARIES))
$(BENCH_BINARIES): $(BUILD_DIR)/% : $(BUILD_SRC_DIR)/Vifm__ALL.a $(BUILD_TEST_DIR)/V%__ALL.a $(BUILD_DIR)/verilator.a $(TEST_LIBRARIES_BINARIES) %.cpp
	$(CXX) $(CFLAGS) $*.cpp -L$(BUILD_SRC_DIR) -L$(BUILD_TEST_DIR) -L$(BUILD_LIBS_DIR) -L$(BUILD_DIR) -l:Vifm__ALL.a -l:V$*__ALL.a -l:verilator.a $(TEST_LIBRARIES_LINK_ARGS) -I$(BUILD_SRC_DIR) -I$(TEST_INC_DIR) -o $@
	cd $(BUILD_DIR) ; $@ 
