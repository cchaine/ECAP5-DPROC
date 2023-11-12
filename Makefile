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

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_DIR              =  build
BUILD_SRC_DIR          =  $(BUILD_DIR)/src/
BUILD_TEST_DIR         =  $(BUILD_DIR)/tests/
BUILD_LIBS_DIR         =  $(BUILD_TEST_DIR)/libs/
SRC_DIR                =  src
INC_DIR                =  $(SRC_DIR)/include
TEST_DIR               =  tests
TEST_INC_DIR           =  $(TEST_DIR)/include
TEST_BENCH_DIR         =  $(TEST_DIR)/benches
TEST_LIBS_DIR 				 =  $(TEST_DIR)/libs

INCLUDES = $(shell find $(INC_DIR) -name '*.svh')
INCLUDES := $(addprefix $(PROJECT_ROOT), $(INCLUDES))

VERILATOR_OPTS = --cc --trace
VERILATOR_WARNINGS = -Wall -Wno-unused -Wno-pinmissing -Wno-caseincomplete

MODULES = regm ifm exm

all: $(MODULES)

clean:
	rm -rf ${BUILD_DIR}

builddir:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_SRC_DIR)
	@mkdir -p $(BUILD_LIBS_DIR)
	@mkdir -p $(BUILD_DIR)/waves

docs: builddir
	cd docs/ && pdflatex --interaction=nonstopmode -halt-on-error -output-directory ../${BUILD_DIR} arch/main.tex 
	cp build/main.pdf docs/arch.pdf
	cd docs/ && pdflatex --interaction=nonstopmode -halt-on-error -output-directory ../${BUILD_DIR} testplan/main.tex 
	cp build/main.pdf docs/arch.pdf

#
# ================ Verilation process for sources ================
# 
$(BUILD_SRC_DIR)/V%__ALL.a: $(BUILD_SRC_DIR)/V%.cpp $(BUILD_SRC_DIR)/V%.h $(BUILD_SRC_DIR)/V%.mk
	make -C $(BUILD_SRC_DIR) -f V$*.mk
$(BUILD_SRC_DIR)/V%.mk: $(INCLUDES) $(SRC_DIR)/%.sv
	verilator $(VERILATOR_OPTS) $(VERILATOR_WARNINGS) --Mdir $(BUILD_SRC_DIR) --prefix V$* -cc $(INCLUDES) $(SRC_DIR)/$*.sv
$(BUILD_SRC_DIR)/V%.cpp $(BUILD_SRC_DIR)/V%.h: $(BUILD_SRC_DIR)/V%.mk ;

#
# ================ Verilation process for test libraries ================
# 
TEST_LIBRARIES = $(basename $(find $(TEST_LIBS_DIR) -name "*.sv"))
TEST_LIBRARIES_BINARIES = $(addsuffix ".a", $(TEST_LIBRARIES))
TEST_LIBRARIES_BINARIES := $(addprefix $(BUILD_TEST_DIR), $(TEST_LIBRARIES_BINARIES))
$(BUILD_LIBS_DIR)/V%__ALL.a: $(BUILD_LIBS_DIR)/V%.cpp $(BUILD_LIBS_DIR)/V%.h $(BUILD_LIBS_DIR)/V%.mk
	make -C $(BUILD_LIBS_DIR) -f V$*.mk
$(BUILD_LIBS_DIR)/V%.mk: $(LIBS_DIR)/%.sv
	verilator $(VERILATOR_OPTS) $(VERILATOR_WARNINGS) --Mdir $(BUILD_LIBS_DIR) --prefix V$* -cc $(LIBS_DIR)/$*.sv
$(BUILD_LIBS_DIR)/V%.cpp $(BUILD_LIBS_DIR)/V%.h: $(BUILD_LIBS_DIR)/V%.mk ;

#
# ================ Compile Verilator C libraries ================
# 
VERILATOR_LIBS = verilated verilated_vcd_c verilated_threads
VERILATOR_LIBS_OBJECTS = $(addsuffix .o, $(VERILATOR_LIBS))
VERILATOR_LIBS_OBJECTS := $(addprefix $(BUILD_DIR)/, $(VERILATOR_LIBS_OBJECTS))
$(BUILD_DIR)/verilator.a: $(VERILATOR_LIBS_OBJECTS)
	ar rcs $@ $(VERILATOR_LIBS_OBJECTS)

$(VERILATOR_LIBS_OBJECTS): $(BUILD_DIR)/%.o : $(VERILATOR_ROOT)/include/%.cpp
	$(CXX) -Wall -Og -g -I$(VERILATOR_ROOT)/include $^ -c -o $@

#
# ================= Test compilation =================
#
$(MODULES): % : builddir $(BUILD_SRC_DIR)/V%__ALL.a $(BUILD_DIR)/verilator.a $(TEST_LIBRARIES_BINARIES)
	make -C $(TEST_BENCH_DIR)/$@ \
		PROJECT_ROOT=$(PROJECT_ROOT) \
		BUILD_DIR=$(PROJECT_ROOT)/$(BUILD_DIR) \
		BUILD_SRC_DIR=$(PROJECT_ROOT)/$(BUILD_SRC_DIR) \
		BUILD_TEST_DIR=$(PROJECT_ROOT)/$(BUILD_TEST_DIR) \
		BUILD_LIBS_DIR=$(PROJECT_ROOT)/$(BUILD_LIBS_DIR) \
		INC_DIR=$(PROJECT_ROOT)/$(INC_DIR) \
		TEST_INC_DIR=$(PROJECT_ROOT)/$(TEST_INC_DIR) \
		TEST_LIBS_DIR=$(PROJECT_ROOT)/$(TEST_LIBS_DIR) 
