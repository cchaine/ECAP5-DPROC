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

module emm import ecap5_dproc_pkg::*;
(
  input   logic        clk_i,
  input   logic        rst_i,

  //=================================
  //    Slave port 1
  
  input   logic[31:0]  s1_wb_adr_i,
  output  logic[31:0]  s1_wb_dat_o,
  input   logic[31:0]  s1_wb_dat_i,
  input   logic        s1_wb_we_i,
  input   logic[3:0]   s1_wb_sel_i,
  input   logic        s1_wb_stb_i,
  output  logic        s1_wb_ack_o,
  input   logic        s1_wb_cyc_i,
  output  logic        s1_wb_stall_o,
  
  //=================================
  //    Slave port 2
  
  input   logic[31:0]  s2_wb_adr_i,
  output  logic[31:0]  s2_wb_dat_o,
  input   logic[31:0]  s2_wb_dat_i,
  input   logic        s2_wb_we_i,
  input   logic[3:0]   s2_wb_sel_i,
  input   logic        s2_wb_stb_i,
  output  logic        s2_wb_ack_o,
  input   logic        s2_wb_cyc_i,
  output  logic        s2_wb_stall_o,

  //=================================
  //    Master port
  
  output  logic[31:0]  m_wb_adr_o,
  input   logic[31:0]  m_wb_dat_i,
  output  logic[31:0]  m_wb_dat_o,
  output  logic        m_wb_we_o,
  output  logic[3:0]   m_wb_sel_o,
  output  logic        m_wb_stb_o,
  input   logic        m_wb_ack_i,
  output  logic        m_wb_cyc_o,
  input   logic        m_wb_stall_i
);

logic switch_d, switch_q;
logic in_request_q;

logic[31:0] sel_wb_adr;
logic[31:0] sel_wb_dat_o;
logic       sel_wb_we;
logic[3:0]  sel_wb_sel;
logic       sel_wb_stb;
logic       sel_wb_ack;
logic       sel_wb_cyc;

always_comb begin
  switch_d = switch_q;

  if(~switch_q) begin
    sel_wb_adr    =  s1_wb_adr_i;
    sel_wb_dat_o  =  s1_wb_dat_i;
    sel_wb_we     =  s1_wb_we_i;
    sel_wb_sel    =  s1_wb_sel_i;
    sel_wb_stb    =  s1_wb_stb_i;
    sel_wb_cyc    =  s1_wb_cyc_i;
  end else begin
    sel_wb_adr    =  s2_wb_adr_i;
    sel_wb_dat_o  =  s2_wb_dat_i;
    sel_wb_we     =  s2_wb_we_i;
    sel_wb_sel    =  s2_wb_sel_i;
    sel_wb_stb    =  s2_wb_stb_i;
    sel_wb_cyc    =  s2_wb_cyc_i;
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    switch_q <= 0;
    in_request_q <= 0;
  end else begin
    if(sel_wb_stb && sel_wb_cyc && m_wb_stall_i == 0) begin
      in_request_q <= 1;
    end
    if(in_request_q && sel_wb_cyc == 0) begin
      in_request_q <= 0;
    end
    switch_q <= switch_d;
  end
end

assign m_wb_adr_o = sel_wb_adr;
assign m_wb_dat_o = sel_wb_dat_o;
assign m_wb_we_o = sel_wb_we;
assign m_wb_sel_o = sel_wb_sel;
assign m_wb_stb_o = sel_wb_stb;
assign m_wb_cyc_o = sel_wb_cyc;

assign s1_wb_stall_o = switch_q;
assign s2_wb_stall_o = ~switch_q;

endmodule // exm
