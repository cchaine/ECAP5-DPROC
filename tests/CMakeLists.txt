#           __        _
#  ________/ /  ___ _(_)__  ___
# / __/ __/ _ \/ _ `/ / _ \/ -_)
# \__/\__/_//_/\_,_/_/_//_/\__/
# 
# Copyright (C) Clément Chain
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

set(BENCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/benches/)
set(TEST_INCLUDE_DIR ${ecap5_dtlib_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/)
set(INCLUDE_DIR ${SRC_DIR}/include/)
set(LIBS_DIR ${ecap5_dtlib_SOURCE_DIR}/src/)
set(TESTDATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/testdata/)
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config/)

# Create folders for the waves and testdata
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/waves)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata)

set(SV_HEADERS ${SRC_DIR}/include/ecap5_dproc_pkg.svh)

# Lint
file(GLOB SRC_FILES ${SRC_DIR}/*.sv)
file(GLOB INCLUDE_FILES ${INCLUDE_DIR}/*.svh)
add_custom_target(lint 
  COMMAND verible-verilog-lint ${SRC_FILES} ${INCLUDE_FILES} --rules_config="${CONFIG_DIR}/verible-lint.rules" --waiver_files="${CONFIG_DIR}/verible-lint.waives"
  DEPENDS ${SRC_FILES} ${INCLUDE_FILES})

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
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
  add_custom_target(${TEST_TARGET} DEPENDS ${TEST_OUTPUT})
  # Add the ${TEST_TARGET} target to the test targets
  list(APPEND TEST_BINARIES ${TARGET})
  list(APPEND TEST_TARGETS ${TEST_TARGET})
  list(APPEND TEST_OUTPUTS ${TEST_OUTPUT})
endmacro()

add_testbench(regs)
add_testbench(fetch)
add_testbench(decode)
add_testbench(execute)
add_testbench(loadstore)
add_testbench(loadstore BENCH loadstore_w_slave LIBS instr_wb_slave)
add_testbench(memory)
add_testbench(hazard)
add_testbench(ecap5_dproc)

# riscv-tests
add_subdirectory(riscv-tests)
add_executable(riscv-tests-executable ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests/riscv-tests.cpp)
target_include_directories(riscv-tests-executable PRIVATE ${TEST_INCLUDE_DIR})
verilate(riscv-tests-executable
  PREFIX Vecap5_dproc
  SOURCES ${SV_HEADERS}
          ${SRC_DIR}/ecap5_dproc.sv
  INCLUDE_DIRS ${SRC_DIR}
  TRACE) 
get_target_property(RISCV_TESTS_EXECUTABLE riscv-tests-executable BINARY_DIR)
if(${DEBUGLOG})
  set(RUN_TARGET_ARGUMENT "-v")  
endif()
add_custom_command(
  COMMAND ${RISCV_TESTS_EXECUTABLE}/riscv-tests-executable ${RUN_TARGET_ARGUMENT}
  OUTPUT ${TESTDATA_DIR}/riscv-tests.csv
  DEPENDS riscv-tests-executable
  WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
add_custom_target(riscv-tests DEPENDS riscv-tests-binaries ${TESTDATA_DIR}/riscv-tests.csv)

# emulator
add_subdirectory(emulator)
add_executable(emulate ${CMAKE_CURRENT_SOURCE_DIR}/emulator/emulator.cpp)
target_include_directories(emulate PRIVATE ${TEST_INCLUDE_DIR})
verilate(emulate
  PREFIX Vecap5_dproc
  SOURCES ${SV_HEADERS}
          ${SRC_DIR}/ecap5_dproc.sv
  INCLUDE_DIRS ${SRC_DIR}
  TRACE) 
add_dependencies(emulate emulator-example-binaries)

# Main targets
add_custom_target(build DEPENDS ${TEST_BINARIES} riscv-tests-executable)
add_custom_target(tests DEPENDS ${TEST_TARGETS} riscv-tests)

