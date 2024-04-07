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

module ifm_with_mem import ecap5_dproc_pkg::*; (
  input   int          testcase,

  input   logic        clk_i,
  input   logic        rst_i,
  input   logic        irq_i,
  input   logic        drq_i,
  input   logic        branch_i,
  input   logic[19:0]  boffset_i,
  input   logic        output_ready_i,
  input   logic        output_valid_o,
  input   logic[31:0]  instr_o,
  input   logic[31:0]  pc_o,
  input   logic        stall_request_i
);

logic[31:0] wb_adr, wb_dat;
logic wb_we, wb_stb, wb_ack, wb_cyc, wb_stall;
logic[3:0] wb_sel;

ifm dut (
  .clk_i           (clk_i),
  .rst_i           (rst_i),
  .irq_i           (irq_i),
  .drq_i           (drq_i),
  .branch_i        (branch_i),
  .boffset_i       (boffset_i),
  .wb_adr_o        (wb_adr),
  .wb_dat_i        (wb_dat),
  .wb_we_o         (wb_we),
  .wb_sel_o        (wb_sel),
  .wb_stb_o        (wb_stb),
  .wb_ack_i        (wb_ack),
  .wb_cyc_o        (wb_cyc),
  .wb_stall_i      (wb_stall),
  .output_ready_i  (output_ready_i),
  .output_valid_o  (output_valid_o),
  .instr_o         (instr_o),
  .pc_o            (pc_o)
);

wishbone_slave mem (
  .clk_i (clk_i),
  .wb_adr_i (wb_adr),
  .wb_dat_o (wb_dat),
  .wb_stb_i (wb_stb),
  .wb_ack_o (wb_ack),
  .wb_cyc_i (wb_cyc),
  .wb_stall_o (wb_stall),
  .stall_request_i (stall_request_i),
);

endmodule // top
