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

module tb_loadstore_w_slave import ecap5_dproc_pkg::*; (
  input   int          testcase,

  input   logic        clk_i,
  input   logic        rst_i,

  //=================================
  //    Input logic
  
  output  logic        input_ready_o,
  input   logic        input_valid_i,

  //`````````````````````````````````
  //    Execute interface 
   
  input   logic[31:0]  alu_result_i,
  input   logic        enable_i,
  input   logic        write_i,
  input   logic[31:0]  write_data_i,
  input   logic[3:0]   sel_i,
  input   logic        unsigned_load_i,

  //`````````````````````````````````
  //    Write-back pass-through
   
  input   logic        reg_write_i,
  input   logic[4:0]   reg_addr_i,

  //=================================
  //    Output logic
  
  output  logic        output_valid_o,

  //`````````````````````````````````
  //    Write-back interface
   
  output  logic        reg_write_o,
  output  logic[4:0]   reg_addr_o,
  output  logic[31:0]  reg_data_o,

  //=================================
  //    Instrumentation input
  
  input logic       stall_request_i,
  input logic[31:0] injected_data_i
);

logic[31:0] wb_adr_o;
logic[31:0] wb_dat_i, wb_dat_o;
logic wb_we_o, wb_stb_o, wb_ack_i, wb_cyc_o, wb_stall_i;
logic[3:0] wb_sel_o;

loadstore dut (
 .clk_i           (clk_i),
 .rst_i           (rst_i),
 .input_ready_o   (input_ready_o),
 .input_valid_i   (input_valid_i),
 .alu_result_i    (alu_result_i),
 .enable_i        (enable_i),
 .write_i         (write_i),
 .write_data_i    (write_data_i),
 .sel_i           (sel_i),
 .unsigned_load_i (unsigned_load_i),
 .reg_write_i     (reg_write_i),
 .reg_addr_i      (reg_addr_i),
 .wb_adr_o        (wb_adr_o),
 .wb_dat_i        (wb_dat_i),
 .wb_dat_o        (wb_dat_o),
 .wb_we_o         (wb_we_o),
 .wb_sel_o        (wb_sel_o),
 .wb_stb_o        (wb_stb_o),
 .wb_ack_i        (wb_ack_i),
 .wb_cyc_o        (wb_cyc_o),
 .wb_stall_i      (wb_stall_i),
 .output_valid_o  (output_valid_o),
 .reg_write_o     (reg_write_o),
 .reg_addr_o      (reg_addr_o),
 .reg_data_o      (reg_data_o)
);

instr_wb_slave wb_slave (
 .clk_i           (clk_i),
 .rst_i           (rst_i),

 .wb_adr_i        (wb_adr_o),
 .wb_dat_o        (wb_dat_i),
 .wb_dat_i        (wb_dat_o),
 .wb_we_i         (wb_we_o),
 .wb_sel_i        (wb_sel_o),
 .wb_stb_i        (wb_stb_o),
 .wb_ack_o        (wb_ack_i),
 .wb_cyc_i        (wb_cyc_o),
 .wb_stall_o      (wb_stall_i),

 .stall_request_i (stall_request_i),
 .injected_data_i (injected_data_i)
);

endmodule // top

`verilator_config

public -module "tb_loadstore_w_slave" -var "wb_adr_o"
public -module "tb_loadstore_w_slave" -var "wb_dat_i"
public -module "tb_loadstore_w_slave" -var "wb_dat_o"
public -module "tb_loadstore_w_slave" -var "wb_we_o"
public -module "tb_loadstore_w_slave" -var "wb_stb_o"
public -module "tb_loadstore_w_slave" -var "wb_ack_i"
public -module "tb_loadstore_w_slave" -var "wb_cyc_o"
public -module "tb_loadstore_w_slave" -var "wb_stall_i"
public -module "tb_loadstore_w_slave" -var "wb_sel_o"
