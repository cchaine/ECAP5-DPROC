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

module tb_memory import ecap5_dproc_pkg::*;
(
  input   int          testcase,

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

memory dut (
  .clk_i (clk_i),
  .rst_i (rst_i),
  .s1_wb_adr_i   (s1_wb_adr_i),
  .s1_wb_dat_o   (s1_wb_dat_o),
  .s1_wb_dat_i   (s1_wb_dat_i),
  .s1_wb_we_i    (s1_wb_we_i),
  .s1_wb_sel_i   (s1_wb_sel_i),
  .s1_wb_stb_i   (s1_wb_stb_i),
  .s1_wb_ack_o   (s1_wb_ack_o),
  .s1_wb_cyc_i   (s1_wb_cyc_i),
  .s1_wb_stall_o (s1_wb_stall_o),

  .s2_wb_adr_i   (s2_wb_adr_i),
  .s2_wb_dat_o   (s2_wb_dat_o),
  .s2_wb_dat_i   (s2_wb_dat_i),
  .s2_wb_we_i    (s2_wb_we_i),
  .s2_wb_sel_i   (s2_wb_sel_i),
  .s2_wb_stb_i   (s2_wb_stb_i),
  .s2_wb_ack_o   (s2_wb_ack_o),
  .s2_wb_cyc_i   (s2_wb_cyc_i),
  .s2_wb_stall_o (s2_wb_stall_o),

  .m_wb_adr_o    (m_wb_adr_o),
  .m_wb_dat_i    (m_wb_dat_i),
  .m_wb_dat_o    (m_wb_dat_o),
  .m_wb_we_o     (m_wb_we_o),
  .m_wb_sel_o    (m_wb_sel_o),
  .m_wb_stb_o    (m_wb_stb_o),
  .m_wb_ack_i    (m_wb_ack_i),
  .m_wb_cyc_o    (m_wb_cyc_o),
  .m_wb_stall_i  (m_wb_stall_i)
);

endmodule // tb_memory
