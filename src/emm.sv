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
  
  output  logic[31:0]  s1_wb_adr_i,
  input   logic[31:0]  s1_wb_dat_o,
  output  logic        s1_wb_we_i,
  output  logic[3:0]   s1_wb_sel_i,
  output  logic        s1_wb_stb_i,
  input   logic        s1_wb_ack_o,
  output  logic        s1_wb_cyc_i,
  input   logic        s1_wb_stall_o,
  
  //=================================
  //    Slave port 2
  
  output  logic[31:0]  s2_wb_adr_i,
  input   logic[31:0]  s2_wb_dat_o,
  output  logic        s2_wb_we_i,
  output  logic[3:0]   s2_wb_sel_i,
  output  logic        s2_wb_stb_i,
  input   logic        s2_wb_ack_o,
  output  logic        s2_wb_cyc_i,
  input   logic        s2_wb_stall_o,

  //=================================
  //    Master port
  
  output  logic[31:0]  m_wb_adr_o,
  input   logic[31:0]  m_wb_dat_i,
  output  logic        m_wb_we_o,
  output  logic[3:0]   m_wb_sel_o,
  output  logic        m_wb_stb_o,
  input   logic        m_wb_ack_i,
  output  logic        m_wb_cyc_o,
  input   logic        m_wb_stall_i,
);

typedef enum logic [2:0] {
  IDLE,           // 0
  RESPONSE,       // 1
  MEMORY_STALL    // 2
} slave_state_t;
slave_state_t s1_state_d, s1_state_q;
slave_state_t s2_state_d, s2_state_q;

logic s1_stall, s2_stall;
assign s1_stall = 1;
assign s2_stall = 1;

typedef enum logic [2:0] {
  IDLE,           // 0
  REQUEST,        // 1
  MEMORY_WAIT,    // 2
  DONE,           // 3
  MEMORY_STALL    // 4
} master_state_t;
state_t m_state_d, m_state_q;

logic memory_request;
assign memory_request = 0;

always_comb begin : slave1_state_machine
  s1_state_d = s1_state_q;

  case(s1_state_q)
    IDLE: begin
      if(s1_wb_stb_i && s1_wb_cyc_o) begin
        if(s1_stall) begin
          s1_state_d = STALL;
        end else begin
          s1_state_d = RESPONSE;
        end
      end
    end
    STALL: begin
      if(!s1_stall) begin
        s1_state_d = RESPONSE;
      end
    end
    RESPONSE: begin
      s1_state_d = IDLE;
    end
    default: begin
    end
  endcase
end

always_comb begin : slave2_state_machine
  s2_state_d = s2_state_q;

  case(s2_state_q)
    IDLE: begin
      if(s2_wb_stb_i && s2_wb_cyc_o) begin
        if(s2_stall) begin
          s2_state_d = STALL;
        end else begin
          s2_state_d = RESPONSE;
        end
      end
    end
    STALL: begin
      if(!s2_stall) begin
        s2_state_d = RESPONSE;
      end
    end
    RESPONSE: begin
      s2_state_d = IDLE;
    end
    default: begin
    end
  endcase
end

always_comb begin : master_state_machine
  m_state_d = m_state_q;

  case(m_state_q)
    IDLE: begin
      if(memory_request) begin
        // A memory request shall be triggered
        if(m_wb_stall_i) begin
          // The memory is stalled
          m_state_d = MEMORY_STALL;
        end else begin
          // The memory is ready
          m_state_d = REQUEST;
        end
      end
    end
    MEMORY_STALL: begin
      if(!m_wb_stall_i) begin
        // The memory is unstalled
        m_state_d = REQUEST;
      end
    end
    REQUEST: begin
      if(m_wb_ack_i) begin
        // The response has been received directly
        m_state_d = DONE;
      end else begin
        // Wait for the response to be received
        m_state_d = MEMORY_WAIT;
      end
    end
    MEMORY_WAIT: begin
      if(m_wb_ack_i) begin
        // The response has been received
        m_state_d = DONE;
      end
    end
    DONE: begin
      m_state_d = IDLE;
    end
    default: begin
    end
  endcase
end

always_ff @(posedge clk_i) begin
  if(rst_i) begin
    s1_state_q <= IDLE;
    s2_state_q <= IDLE;
    m_state_q <= IDLE;
  end else begin
    s1_state_q <= s1_state_d; 
    s2_state_q <= s2_state_d; 
    m_state_q <= m_state_d;
  end
end

endmodule // exm
