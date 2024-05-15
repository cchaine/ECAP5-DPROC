#           __        _
#  ________/ /  ___ _(_)__  ___
# / __/ __/ _ \/ _ `/ / _ \/ -_)
# \__/\__/_//_/\_,_/_/_//_/\__/
# 
# Copyright (C) Clément Chaine
# This file is part of ECAP5-DPROC <https://github.com/ecap5/ECAP5-DPROC>
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

set(SV_HEADERS ${SRC_DIR}/include/ecap5_dproc_pkg.svh
               ${SRC_DIR}/include/riscv_pkg.svh)

# Testbench
set(TEST_BINARIES)
set(TEST_TARGETS)
set(TEST_OUTPUTS)
macro(add_testbench module)
  cmake_parse_arguments(ARG ""
                            "BENCH"
                            "LIBS"
                            ${ARGN})

  if("${ARG_BENCH}" STREQUAL "")
    set(ARG_BENCH ${module})
  endif()
  set(TARGET tb_${ARG_BENCH})

  # Make a file list for testing libraries
  list(TRANSFORM ARG_LIBS APPEND ".sv")
  list(TRANSFORM ARG_LIBS PREPEND ${LIBS_DIR})

  # Create the test executable
  add_executable(${TARGET} ${BENCH_DIR}/${module}/${TARGET}.cpp)
  target_include_directories(${TARGET} PUBLIC ${TEST_INCLUDE_DIR})
  verilate(${TARGET}
    PREFIX V${TARGET}
    SOURCES ${SV_HEADERS}
            ${BENCH_DIR}/${module}/${TARGET}.sv
            ${ARG_LIBS}
    INCLUDE_DIRS ${SRC_DIR}
    TRACE)

  set(TEST_TARGET simulate_${ARG_BENCH})
  set(TEST_OUTPUT ${TESTDATA_DIR}/${ARG_BENCH}.csv)
  # Create the ${TEST_TARGET} command to run the test
  get_target_property(BINARY_DIR "${TARGET}" BINARY_DIR)
  if(${DEBUGLOG})
    set(RUN_TARGET_ARGUMENT "-v")  
  endif()
  add_custom_command(
    COMMAND ${TARGET} ${RUN_TARGET_ARGUMENT}
    OUTPUT ${TEST_OUTPUT}
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/)
  add_custom_target(${TEST_TARGET} DEPENDS ${TEST_OUTPUT})
  # Add the ${TEST_TARGET} target to the test targets
  list(APPEND TEST_BINARIES ${TARGET})
  list(APPEND TEST_TARGETS ${TEST_TARGET})
  list(APPEND TEST_OUTPUTS ${TEST_OUTPUT})
endmacro()

add_testbench(registers)
add_testbench(fetch)
add_testbench(decode)
add_testbench(execute)
add_testbench(loadstore)
add_testbench(loadstore BENCH loadstore_w_slave LIBS instr_wb_slave)
add_testbench(writeback)
add_testbench(memory)
add_testbench(hazard)
add_testbench(ecap5_dproc)

add_custom_target(benches-build DEPENDS ${TEST_BINARIES})
add_custom_target(benches DEPENDS ${TEST_TARGETS})
