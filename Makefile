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

BUILD_DIR  =  build
SRC_DIR    =  src
TEST_DIR   =  tests

SOURCES = $(shell find $(SRC_DIR) -name '*.v')
SOURCES := $(addprefix $(PROJECT_ROOT), $(SOURCES))

TOP_MODULE = top
MODULES = regm ifm

VERILATOR_OPTS = --cc --trace
VERILATOR_WARNINGS = -Wall -Wno-unused -Wno-pinmissing

builddir:
	mkdir -p ${BUILD_DIR}
	mkdir -p ${BUILD_DIR}/waves

docs: builddir
	cd docs/ && pdflatex --interaction=nonstopmode -halt-on-error -output-directory ../${BUILD_DIR} arch/main.tex 
	cp build/main.pdf docs/arch.pdf
	cd docs/ && pdflatex --interaction=nonstopmode -halt-on-error -output-directory ../${BUILD_DIR} testplan/main.tex 
	cp build/main.pdf docs/arch.pdf

synth:
	yosys -s config/synth.conf

lint:
	verilator \
		--lint-only \
		${VERILATOR_WARNINGS} \
		${SOURCES} \
		--top-module ${TOP_MODULE}

tests: $(MODULES)

$(MODULES): builddir
	@echo "Testing $@..."
	verilator \
		${VERILATOR_OPTS} ${VERILATOR_WARNINGS} \
		--Mdir $(BUILD_DIR)/ \
		-exe $(SRC_DIR)/$@.v $(PROJECT_ROOT)$(TEST_DIR)/tb_$@.cpp \
		--top-module $@ --prefix V$@ \
		-CFLAGS -g
	make -C $(BUILD_DIR) -f V$@.mk V$@ > /dev/null
	cd $(BUILD_DIR)/ && ./V$@
	
