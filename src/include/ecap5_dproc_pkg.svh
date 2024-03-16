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

localparam  logic[31:0]  boot_address       /* verilator public */ =  32'h00000000;
localparam  logic[31:0]  interrupt_address  /* verilator public */ =  32'hFF00000A;
localparam  logic[31:0]  debug_address      /* verilator public */ =  32'hFF00000B;

localparam  logic[2:0]  ALU_ADD    /* verilator public */ = 3'h0;
localparam  logic[2:0]  ALU_XOR    /* verilator public */ = 3'h1;
localparam  logic[2:0]  ALU_OR     /* verilator public */ = 3'h2;
localparam  logic[2:0]  ALU_AND    /* verilator public */ = 3'h3;
localparam  logic[2:0]  ALU_SLT    /* verilator public */ = 3'h4;
localparam  logic[2:0]  ALU_SLTU   /* verilator public */ = 3'h5;
localparam  logic[2:0]  ALU_SHIFT  /* verilator public */ = 3'h6;

localparam  logic[2:0]  NO_BRANCH    /* verilator public */ = 3'h0;
localparam  logic[2:0]  BRANCH_BEQ   /* verilator public */ = 3'h1;
localparam  logic[2:0]  BRANCH_BNE   /* verilator public */ = 3'h2;
localparam  logic[2:0]  BRANCH_BLT   /* verilator public */ = 3'h3;
localparam  logic[2:0]  BRANCH_BLTU  /* verilator public */ = 3'h4;
localparam  logic[2:0]  BRANCH_BGE   /* verilator public */ = 3'h5;
localparam  logic[2:0]  BRANCH_BGEU  /* verilator public */ = 3'h6;

endpackage
