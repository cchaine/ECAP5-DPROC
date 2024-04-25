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

module tb_ecap5_dproc (
  input   int          testcase,

  input  logic        clk_i,
  input  logic        rst_i,

  input  logic        irq_i,
  input  logic        drq_i,

  output logic[31:0]  wb_adr_o,
  input  logic[31:0]  wb_dat_i,
  output logic[31:0]  wb_dat_o,
  output logic[3:0]   wb_sel_o,
  output logic        wb_we_o,
  output logic        wb_stb_o,
  input  logic        wb_ack_i,
  output logic        wb_cyc_o,
  input  logic        wb_stall_i
);

ecap5_dproc dut (
  .clk_i      (clk_i),
  .rst_i      (rst_i),

  .irq_i      (irq_i),
  .drq_i      (drq_i),

  .wb_adr_o   (wb_adr_o),
  .wb_dat_i   (wb_dat_i),
  .wb_dat_o   (wb_dat_o),
  .wb_sel_o   (wb_sel_o),
  .wb_we_o    (wb_we_o),
  .wb_stb_o   (wb_stb_o),
  .wb_ack_i   (wb_ack_i),
  .wb_cyc_o   (wb_cyc_o),
  .wb_stall_i (wb_stall_i)
);

endmodule // ecap5_dproc
