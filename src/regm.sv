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

module regm (
  input   logic        clk_i,     
  // First reading port
  input   logic[4:0]   raddr1_i,  
  output  logic[31:0]  rdata1_o,  
  // Second reading port
  input   logic[4:0]   raddr2_i,  
  output  logic[31:0]  rdata2_o,  
  // Writing port       
  input   logic        write_i,   
  input   logic[4:0]   waddr_i,   
  input   logic[31:0]  wdata_i    
);

logic[31:0] registers [31:0];

always @ (posedge clk_i) begin
  if (write_i & (waddr_i != 0)) begin
    registers[waddr_i] <= wdata_i;
  end else begin
    registers[waddr_i] <= registers[waddr_i];
  end

  rdata1_o <= registers[raddr1_i];
  rdata2_o <= registers[raddr2_i];
end

`ifdef VERILATOR
  export "DPI-C" task set_register_value;
  task set_register_value(input logic[4:0] addr, input logic[31:0] value);
    registers[addr] = value;
  endtask
`endif

endmodule // regm
