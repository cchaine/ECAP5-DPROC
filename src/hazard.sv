
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
  output  logic  control_discard_o
);

logic[1:0] control_discard_ctr_d, control_discard_ctr_q;
logic control_hazard;

assign control_hazard = control_discard_ctr_q > 0;

always_comb begin
  control_discard_ctr_d = control_discard_ctr_q;
  if(branch_i) begin
    control_discard_ctr_d = 2;
  end else begin
    if(control_hazard) begin
      control_discard_ctr_d = control_discard_ctr_q - 1;
    end
  end
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    control_discard_ctr_q <= '0;
  end else begin
    control_discard_ctr_q <= control_discard_ctr_d;
  end
end

assign control_discard_o = control_hazard;

endmodule // hazard
