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

include(${CMAKE_SOURCE_DIR}/cmake/riscv.cmake)

enable_language(ASM)

set(TVM rv32ui) # Target Virtual Machine
set(TE p)       # Target Environment

# Paths
set(SOURCE_DIR ${riscv_tests_SOURCE_DIR}/isa/${TVM}/)
set(ENV_DIR ${riscv_tests_SOURCE_DIR}/env/${TE}/)
set(INCLUDE_DIRS ${ENV_DIR} 
                 ${riscv_tests_SOURCE_DIR}/isa/macros/scalar)

# Compile options
set(CC_OPTS -static
            -mcmodel=medany
            -fvisibility=hidden
            -nostdlib
            -nostartfiles)

set(TARGETS simple
            add addi 
            and andi
            auipc
            beq bge bgeu blt bltu bne
            fence_i
            jal jalr
            lb lbu lh lhu lw
            lui
            ma_data
            or ori
            sb sh sw
            sll slli
            slt slti
            sltiu sltu
            sra srai
            srl srli
            sub
            xor xori)

set(ALL_TARGETS)
foreach(TARGET IN LISTS TARGETS)
  set(FULL_TARGET ${TVM}-${TE}-${TARGET})
  add_executable(${FULL_TARGET} ${SOURCE_DIR}/${TARGET}.S)
  add_compile_options(${CC_OPTS} -march=rv32g -mabi=ilp32)
  target_include_directories(${FULL_TARGET} PRIVATE ${INCLUDE_DIRS})
  target_link_options(${FULL_TARGET} PRIVATE -T${ENV_DIR}/link.ld)

  list(APPEND ALL_TARGETS ${FULL_TARGET})
endforeach()

add_custom_target(riscv-tests DEPENDS ${ALL_TARGETS})
