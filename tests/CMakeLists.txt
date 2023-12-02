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
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/)

set(SV_HEADERS ${SRC_DIR}/include/ecap5_dproc_pkg.svh)

set(TEST_TARGETS)

macro(add_testbench module)
  set(TARGET tb_${module})
  cmake_parse_arguments(TESTBENCH ""
                                  ""
                                  "LIBS"
                                  ${ARGN})
  # Make a file list for testing libraries
  list(TRANSFORM TESTBENCH_LIBS APPEND ".sv")
  list(TRANSFORM TESTBENCH_LIBS PREPEND ${LIBS_DIR})
  # Create the test executable
  add_executable(${TARGET} ${BENCH_DIR}/${module}/${TARGET}.cpp)
  target_include_directories(${TARGET} PUBLIC ${INCLUDE_DIR})
  verilate(${TARGET}
    PREFIX V${TARGET}
    SOURCES ${SV_HEADERS}
            ${BENCH_DIR}/${module}/${TARGET}.sv
            ${TESTBENCH_LIBS}
    INCLUDE_DIRS ${SRC_DIR}
    TRACE)

  set(TEST_TARGET simulate_${module})
  # Create the ${TEST_TARGET} command to run the test
  get_target_property(BINARY_DIR "${TARGET}" BINARY_DIR)
  add_custom_target(${TEST_TARGET}
    COMMAND ${TARGET}
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
  # Add the ${TEST_TARGET} target to the test targets
  list(APPEND TEST_TARGETS ${TEST_TARGET})
endmacro()

add_testbench(regm)
add_testbench(ifm LIBS wishbone_slave)

add_custom_target(tests)
add_dependencies(tests ${TEST_TARGETS})

# Create folders for the waves and testdata
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/waves)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata)
