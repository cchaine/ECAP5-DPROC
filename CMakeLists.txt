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

cmake_minimum_required(VERSION 3.12)
include(FetchContent)

project(ECAP5_DPROC VERSION 1.0)

# Find packages
find_package(verilator HINTS $ENV{VERILATOR_ROOT})

# Download external dependencies
FetchContent_Declare(ECAP5_TREQ
  GIT_REPOSITORY https://github.com/cchaine/ECAP5-TREQ  
  GIT_TAG origin/main
)
FetchContent_MakeAvailable(ECAP5_TREQ)

add_subdirectory(tests)

# Define commands for using ECAP5-TREQ
add_custom_command(
  OUTPUT test-report.html
  COMMAND python3 ${ecap5_treq_SOURCE_DIR}/src/ecap5_treq/main.py gen_test_report -s ${CMAKE_SOURCE_DIR}/docs/arch/content -m ${CMAKE_SOURCE_DIR}/traceability/matrix.csv -d . -o test-report.html)
add_custom_target(test_report DEPENDS test-report.html)

add_custom_command(
  OUTPUT trace-report.html
  COMMAND python3 ${ecap5_treq_SOURCE_DIR}/src/ecap5_treq/main.py gen_trace_report -s ${CMAKE_SOURCE_DIR}/docs/arch/content -m ${CMAKE_SOURCE_DIR}/traceability/matrix.csv -d . -o trace-report.html)
add_custom_target(trace_report DEPENDS trace-report.html)
