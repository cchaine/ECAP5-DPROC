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

# Lint
file(GLOB SRC_FILES ${SRC_DIR}/*.sv)
file(GLOB INCLUDE_FILES ${INCLUDE_DIR}/*.svh)
add_custom_target(lint 
  COMMAND verible-verilog-lint ${SRC_FILES} ${INCLUDE_FILES} --rules_config="${CONFIG_DIR}/verible-lint.rules" --waiver_files="${CONFIG_DIR}/verible-lint.waives"
  DEPENDS ${SRC_FILES} ${INCLUDE_FILES})

add_subdirectory(benches)
add_subdirectory(emulator)
add_subdirectory(riscv-tests)

# Main targets
add_custom_target(build DEPENDS benches-build riscv-tests-executable)
add_custom_target(tests DEPENDS benches riscv-tests)

