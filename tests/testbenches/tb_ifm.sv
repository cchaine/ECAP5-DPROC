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

module tb_ifm (
  input   logic        clk_i,
  input   logic        rst_i,
  input   logic        irq_i,
  input   logic        drq_i,
  input   logic        branch_i,
  input   logic[19:0]  boffset_i,
  input   logic        output_ready_i,
  output  logic        output_valid_o,
  output  logic[31:0]  instr_o,
  input   logic        stall_request_i
);

logic[31:0]  wb_adr_o   /* verilator public */ ; 
logic[31:0]  wb_dat_i   /* verilator public */ ; 
logic        wb_stb_o   /* verilator public */ ; 
logic        wb_ack_i   /* verilator public */ ; 
logic        wb_cyc_o   /* verilator public */ ; 
logic        wb_stall_i /* verilator public */ ;

ifm dut (
  .clk_i           (clk_i),
  .rst_i           (rst_i),
  .irq_i           (irq_i),
  .drq_i           (drq_i),
  .branch_i        (branch_i),
  .boffset_i       (boffset_i),
  .wb_adr_o        (wb_adr_o),
  .wb_dat_i        (wb_dat_i),
  .wb_stb_o        (wb_stb_o),
  .wb_ack_i        (wb_ack_i),
  .wb_cyc_o        (wb_cyc_o),
  .wb_stall_i      (wb_stall_i),
  .output_ready_i  (output_ready_i),
  .output_valid_o  (output_valid_o),
  .instr_o         (instr_o)
);

wishbone_slave mem (
  .clk_i  (clk_i),
  .wb_adr_i        ({2'b0, wb_adr_o[31:2]}),
  .wb_dat_o        (wb_dat_i),
  .wb_stb_i        (wb_stb_o),
  .wb_ack_o        (wb_ack_i),
  .wb_cyc_i        (wb_cyc_o),
  .wb_stall_o      (wb_stall_i),
  .stall_request_i (stall_request_i)
);

endmodule // top
