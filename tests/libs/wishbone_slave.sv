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

module wishbone_slave (
  input   logic        clk_i,
  input   logic[31:0]  wb_adr_i,
  output  logic[31:0]  wb_dat_o,
  input   logic        wb_stb_i,
  output  logic        wb_ack_o,
  input   logic        wb_cyc_i,
  output  logic        wb_stall_o,
  input   logic        stall_request_i,
  input   logic[31:0]  injected_data_i
);

enum logic[1:0] {
  WAITREQ,
  STALL, 
  RESPONSE
} state_d, state_q;

always_comb begin
  state_d = state_q;
  wb_ack_o = 0;
  case(state_q)
    WAITREQ: begin
      if(wb_stb_i) begin
        if(stall_request_i) begin
          state_d = STALL;
        end else begin
          state_d = RESPONSE;
        end
      end
    end
    STALL: begin
      if(!stall_request_i) begin
        state_d = RESPONSE;
      end
    end
    RESPONSE: begin
      state_d = WAITREQ;
      wb_ack_o = 1;
    end
    default: begin
    end
  endcase
end

always_ff @(posedge clk_i) begin
  state_q  <=  state_d;
end

assign wb_stall_o = stall_request_i;
assign wb_dat_o = injected_data_i;

endmodule
