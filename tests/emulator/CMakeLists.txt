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

set(CROSS_COMPILE riscv64-unknown-elf-)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_AR ${CROSS_COMPILE}ar)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

enable_language(ASM)

# Compile options
set(CC_OPTS "-static \
            -mcmodel=medany \
            -fvisibility=hidden \
            -nostdlib \
            -nostartfiles")

set(TARGETS helloworld)

set(ALL_TARGETS)
foreach(TARGET IN LISTS TARGETS)
  add_executable(${TARGET}.elf ${CMAKE_CURRENT_LIST_DIR}/examples/${TARGET}.s)
  set_target_properties(${TARGET}.elf PROPERTIES COMPILE_FLAGS "${CC_OPTS} -march=rv32g -mabi=ilp32"
                                                 LINK_FLAGS    "${CC_OPTS} -march=rv32g -mabi=ilp32 -T${CMAKE_CURRENT_LIST_DIR}/env/link.ld")

  add_custom_command(
    OUTPUT ${TARGET}.dump
    COMMAND ${CROSS_COMPILE}objdump -d ${TARGET}.elf -Mno-aliases -Mnumeric > ${TARGET}.dump
    VERBATIM)

  list(APPEND ALL_TARGETS ${TARGET}.elf ${TARGET}.dump)
endforeach()

add_custom_target(emulator-example-binaries DEPENDS ${ALL_TARGETS})
