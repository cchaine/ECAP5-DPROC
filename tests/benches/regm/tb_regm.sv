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

module tb_regm (
  input   int          testcase,

  input   logic        clk_i,     
  input   logic[4:0]   raddr1_i,  
  output  logic[31:0]  rdata1_o,  
  input   logic[4:0]   raddr2_i,  
  output  logic[31:0]  rdata2_o,  
  input   logic        write_i,   
  input   logic[4:0]   waddr_i,   
  input   logic[31:0]  wdata_i    
);

regm dut (
  .clk_i     (clk_i),
  .raddr1_i  (raddr1_i),
  .rdata1_o  (rdata1_o),
  .raddr2_i  (raddr2_i),
  .rdata2_o  (rdata2_o),
  .write_i   (write_i),
  .waddr_i   (waddr_i),
  .wdata_i   (wdata_i)   
);

endmodule // top
