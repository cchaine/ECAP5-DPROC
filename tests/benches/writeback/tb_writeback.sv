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

module tb_writeback import ecap5_dproc_pkg::*;
(
  input   int          testcase,

  input   logic        clk_i,     
  input   logic        rst_i,

  input   logic        input_valid_i,

  input   logic        reg_write_i,   
  input   logic[4:0]   reg_addr_i,   
  input   logic[31:0]  reg_data_i,

  output  logic        reg_write_o,   
  output  logic[4:0]   reg_addr_o,   
  output  logic[31:0]  reg_data_o    
);

writeback dut (
  .clk_i (clk_i),
  .rst_i (rst_i),

  .input_valid_i (input_valid_i),
  
  .reg_write_i (reg_write_i),
  .reg_addr_i  (reg_addr_i),
  .reg_data_i  (reg_data_i),

  .reg_write_o (reg_write_o),
  .reg_addr_o  (reg_addr_o),
  .reg_data_o  (reg_data_o)
);

endmodule // tb_writeback
