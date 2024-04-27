
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

module hazard import ecap5_dproc_pkg::*;
(
  input   logic         clk_i,
  input   logic         rst_i,

  input   logic  branch_i,
  output  logic  ex_discard_request_o,

  input   logic[4:0] reg_raddr1_i,
  input   logic[4:0] reg_raddr2_i,
  input   logic      dec_reg_write_i,
  input   logic[4:0] dec_reg_addr_i,
  input   logic      ex_reg_write_i,
  input   logic[4:0] ex_reg_addr_i,
  input   logic      ls_reg_write_i,
  input   logic[4:0] ls_reg_addr_i,
  input   logic      reg_write_i,
  input   logic[4:0] reg_waddr_i,
  output  logic      dec_stall_request_o
);

logic branch_q;
logic control_hazard;
logic dec_data_hazard, ex_data_hazard, ls_data_hazard, rw_data_hazard;

assign dec_data_hazard = dec_reg_write_i && ((reg_raddr1_i != 5'h0) && (reg_raddr1_i == dec_reg_addr_i) || 
                                            ((reg_raddr2_i != 5'h0) && (reg_raddr2_i == dec_reg_addr_i)));
assign ex_data_hazard  = ex_reg_write_i  && ((reg_raddr1_i != 5'h0) && (reg_raddr1_i == ex_reg_addr_i) || 
                                            ((reg_raddr2_i != 5'h0) && (reg_raddr2_i == ex_reg_addr_i)));
assign ls_data_hazard  = ls_reg_write_i  && ((reg_raddr1_i != 5'h0) && (reg_raddr1_i == ls_reg_addr_i) || 
                                            ((reg_raddr2_i != 5'h0) && (reg_raddr2_i == ls_reg_addr_i)));
  assign rw_data_hazard  = reg_write_i; //    && ((reg_raddr1_i != 5'h0) && (reg_raddr1_i == reg_waddr_i) || 
                                     //       ((reg_raddr2_i != 5'h0) && (reg_raddr2_i == reg_waddr_i)));

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    branch_q <= 0;
  end else begin
    branch_q <= branch_i;
  end
end

assign ex_discard_request_o = branch_i || branch_q;

assign dec_stall_request_o = dec_data_hazard || ex_data_hazard || ls_data_hazard || rw_data_hazard;

endmodule // hazard
