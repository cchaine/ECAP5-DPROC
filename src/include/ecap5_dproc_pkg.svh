/*           __        _
 *  ________/ /  ___ _(_)__  ___
 * / __/ __/ _ \/ _ `/ / _ \/ -_)
 * \__/\__/_//_/\_,_/_/_//_/\__/
 * 
 * Copyright (C) Clément Chaine
 * This file is part of ECAP5-DPROC <https://github.com/cchaine/ECAP5-DPROC>
 *
 * ECAP5-DPROC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ECAP5-DPROC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ECAP5-DPROC.  If not, see <http://www.gnu.org/licenses/>.
 */

package ecap5_dproc_pkg;

localparam  logic[31:0]  boot_address       =  32'h00000000;
localparam  logic[31:0]  interrupt_address  =  32'hFF00000A;
localparam  logic[31:0]  debug_address      =  32'hFF00000B;

typedef enum logic[5:0] {
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BLTU, BGE, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADD, ADDI, SUB, XOR, XORI, OR, ORI, AND, ANDI, SLT, SLTI, SLTU, SLTIU, SLL, SLLI, SRL, SRLI, SRA, SRAI
} instr_t;

endpackage
