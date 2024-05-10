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

foreach(TARGET IN LISTS TARGETS)
  add_executable(${TARGET}.elf ${CMAKE_CURRENT_LIST_DIR}/${TARGET}.S)
  set_target_properties(${TARGET}.elf PROPERTIES COMPILE_FLAGS "${CC_OPTS} -march=rv32g -mabi=ilp32"
                                                 LINK_FLAGS    "${CC_OPTS} -march=rv32g -mabi=ilp32 -T${CMAKE_CURRENT_LIST_DIR}/link.ld")

  add_custom_command(
    OUTPUT ${TARGET}.dump
    COMMAND ${CROSS_COMPILE}objdump -d ${TARGET}.elf -Mno-aliases -Mnumeric > ${TARGET}.dump
    VERBATIM)

  get_target_property(EMULATOR_PATH emulator BINARY_DIR)
  add_custom_target(emulate_${TARGET}
    COMMAND ${EMULATOR_PATH}/emulator ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.elf
    DEPENDS emulator ${TARGET}.elf ${TARGET}.dump)
endforeach()
