/*           __        _
 *  ________/ /  ___ _(_)__  ___
 * / __/ __/ _ \/ _ `/ / _ \/ -_)
 * \__/\__/_//_/\_,_/_/_//_/\__/
 * 
 * Copyright (C) Clément Chaine
 * This file is part of ECAP5-DPROC <https://github.com/ecap5/ECAP5-DPROC>
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

localparam  logic[31:0]  BOOT_ADDRESS       /* verilator public */ =  32'h00000000;
localparam  logic[31:0]  INTERRUPT_ADDRESS  /* verilator public */ =  32'hFF00000A;

localparam  logic[2:0]  ALU_ADD    /* verilator public */ = 3'h0;
localparam  logic[2:0]  ALU_XOR    /* verilator public */ = 3'h1;
localparam  logic[2:0]  ALU_OR     /* verilator public */ = 3'h2;
localparam  logic[2:0]  ALU_AND    /* verilator public */ = 3'h3;
localparam  logic[2:0]  ALU_SLT    /* verilator public */ = 3'h4;
localparam  logic[2:0]  ALU_SLTU   /* verilator public */ = 3'h5;
localparam  logic[2:0]  ALU_SHIFT  /* verilator public */ = 3'h6;

localparam  logic[2:0]  NO_BRANCH      /* verilator public */ = 3'h0;
localparam  logic[2:0]  BRANCH_BEQ     /* verilator public */ = 3'h1;
localparam  logic[2:0]  BRANCH_BNE     /* verilator public */ = 3'h2;
localparam  logic[2:0]  BRANCH_BLT     /* verilator public */ = 3'h3;
localparam  logic[2:0]  BRANCH_BLTU    /* verilator public */ = 3'h4;
localparam  logic[2:0]  BRANCH_BGE     /* verilator public */ = 3'h5;
localparam  logic[2:0]  BRANCH_BGEU    /* verilator public */ = 3'h6;
localparam  logic[2:0]  BRANCH_UNCOND  /* verilator public */ = 3'h7;


localparam  logic[6:0]  OPCODE_LUI    /* verilator public */ = 7'b0110111;
localparam  logic[6:0]  OPCODE_AUIPC  /* verilator public */ = 7'b0010111;
localparam  logic[6:0]  OPCODE_OP     /* verilator public */ = 7'b0110011;
localparam  logic[6:0]  OPCODE_OP_IMM /* verilator public */ = 7'b0010011;
localparam  logic[6:0]  OPCODE_JAL    /* verilator public */ = 7'b1101111;
localparam  logic[6:0]  OPCODE_JALR   /* verilator public */ = 7'b1100111;
localparam  logic[6:0]  OPCODE_BRANCH /* verilator public */ = 7'b1100011;
localparam  logic[6:0]  OPCODE_LOAD   /* verilator public */ = 7'b0000011;
localparam  logic[6:0]  OPCODE_STORE  /* verilator public */ = 7'b0100011;

localparam  logic[2:0]  FUNC3_JALR    /* verilator public */ = 3'b000;
localparam  logic[2:0]  FUNC3_BEQ     /* verilator public */ = 3'b000;
localparam  logic[2:0]  FUNC3_BNE     /* verilator public */ = 3'b001;
localparam  logic[2:0]  FUNC3_BLT     /* verilator public */ = 3'b100;
localparam  logic[2:0]  FUNC3_BGE     /* verilator public */ = 3'b101;
localparam  logic[2:0]  FUNC3_BLTU    /* verilator public */ = 3'b110;
localparam  logic[2:0]  FUNC3_BGEU    /* verilator public */ = 3'b111;
localparam  logic[2:0]  FUNC3_LB      /* verilator public */ = 3'b000;
localparam  logic[2:0]  FUNC3_LH      /* verilator public */ = 3'b001;
localparam  logic[2:0]  FUNC3_LW      /* verilator public */ = 3'b010;
localparam  logic[2:0]  FUNC3_LBU     /* verilator public */ = 3'b100;
localparam  logic[2:0]  FUNC3_LHU     /* verilator public */ = 3'b101;
localparam  logic[2:0]  FUNC3_SB      /* verilator public */ = 3'b000;
localparam  logic[2:0]  FUNC3_SH      /* verilator public */ = 3'b001;
localparam  logic[2:0]  FUNC3_SW      /* verilator public */ = 3'b010;
localparam  logic[2:0]  FUNC3_ADD     /* verilator public */ = 3'b000;
localparam  logic[2:0]  FUNC3_SLT     /* verilator public */ = 3'b010;
localparam  logic[2:0]  FUNC3_SLTU    /* verilator public */ = 3'b011;
localparam  logic[2:0]  FUNC3_XOR     /* verilator public */ = 3'b100;
localparam  logic[2:0]  FUNC3_OR      /* verilator public */ = 3'b110;
localparam  logic[2:0]  FUNC3_AND     /* verilator public */ = 3'b111;
localparam  logic[2:0]  FUNC3_SLL     /* verilator public */ = 3'b001;
localparam  logic[2:0]  FUNC3_SRL     /* verilator public */ = 3'b101;

localparam  logic[6:0]  FUNC7_ADD     /* verilator public */ = 7'b0000000;
localparam  logic[6:0]  FUNC7_SUB     /* verilator public */ = 7'b0100000;
localparam  logic[6:0]  FUNC7_SRL     /* verilator public */ = 7'b0000000;
localparam  logic[6:0]  FUNC7_SRA     /* verilator public */ = 7'b0100000;

endpackage
