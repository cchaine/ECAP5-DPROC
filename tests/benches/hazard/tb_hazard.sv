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

module tb_hazard import ecap5_dproc_pkg::*;
(
  input   int          testcase,

  input   logic         clk_i,
  input   logic         rst_i,

  input   logic  branch_i,
  output  logic  ex_discard_o,

  input   logic[4:0] dec_raddr1_i,
  input   logic[4:0] dec_raddr2_i,
  input   logic      ex_reg_write_i,
  input   logic[4:0] ex_reg_addr_i,
  input   logic      ls_reg_write_i,
  input   logic[4:0] ls_reg_addr_i,
  input   logic      rw_reg_write_i,
  input   logic[4:0] rw_reg_addr_i,
  output  logic      dec_stall_request_o
);

hazard dut (
  .clk_i (clk_i),
  .rst_i (rst_i),

  .branch_i (branch_i),
  .ex_discard_o (ex_discard_o),

  .dec_raddr1_i         (dec_raddr1_i),
  .dec_raddr2_i         (dec_raddr2_i),
  .ex_reg_write_i       (ex_reg_write_i),
  .ex_reg_addr_i        (ex_reg_addr_i),
  .ls_reg_write_i       (ls_reg_write_i),
  .ls_reg_addr_i        (ls_reg_addr_i),
  .rw_reg_write_i       (rw_reg_write_i),
  .rw_reg_addr_i        (rw_reg_addr_i),
  .dec_stall_request_o  (dec_stall_request_o)
);

endmodule // tb_hazard
