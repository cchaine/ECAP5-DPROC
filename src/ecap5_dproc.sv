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

module ecap5_dproc #(
  parameter logic[31:0] BOOT_ADDRESS      = 32'h00001000,
  parameter logic[31:0] INTERRUPT_ADDRESS = 32'h00000000
)(
  input  logic        clk_i,
  input  logic        rst_i,

  input  logic        irq_i,

  output logic[31:0]  wb_adr_o,
  input  logic[31:0]  wb_dat_i,
  output logic[31:0]  wb_dat_o,
  output logic[3:0]   wb_sel_o,
  output logic        wb_we_o,
  output logic        wb_stb_o,
  input  logic        wb_ack_i,
  output logic        wb_cyc_o,
  input  logic        wb_stall_i
);

// registers interface
logic[4:0]  reg_raddr1, reg_raddr2, reg_waddr;
logic[31:0] reg_rdata1, reg_rdata2, reg_wdata;
logic       reg_write;

// branch interface
logic       branch;
logic[31:0] branch_target;

// fetch wishbone
logic[31:0]  if_wb_adr_o;
logic[31:0]  if_wb_dat_i;
logic        if_wb_we_o;
logic[3:0]   if_wb_sel_o;
logic        if_wb_stb_o;
logic        if_wb_ack_i;
logic        if_wb_cyc_o;
logic        if_wb_stall_i;

// fetch output
logic[31:0] if_instr;
logic[31:0] if_pc;

// decode output
logic[31:0]  dec_pc;
logic[31:0]  dec_alu_operand1;
logic[31:0]  dec_alu_operand2;
logic[2:0]   dec_alu_op;
logic        dec_alu_sub;
logic        dec_alu_shift_left;
logic        dec_alu_signed_shift;
logic[2:0]   dec_branch_cond;
logic[19:0]  dec_branch_offset;
logic        dec_reg_write;
logic[4:0]   dec_reg_addr;
logic        dec_ls_enable;
logic        dec_ls_write;
logic[31:0]  dec_ls_write_data;
logic[3:0]   dec_ls_sel;
logic        dec_ls_unsigned_load;

// execute output
logic[31:0] ex_result;
logic       ex_ls_enable;
logic       ex_ls_write;
logic[31:0] ex_ls_write_data;
logic[3:0]  ex_ls_sel;
logic       ex_ls_unsigned_load;
logic       ex_reg_write;
logic[4:0]  ex_reg_addr;

// loadstore wishbone
logic[31:0]  ls_wb_adr_o;
logic[31:0]  ls_wb_dat_i;
logic[31:0]  ls_wb_dat_o;
logic        ls_wb_we_o;
logic[3:0]   ls_wb_sel_o;
logic        ls_wb_stb_o;
logic        ls_wb_ack_i;
logic        ls_wb_cyc_o;
logic        ls_wb_stall_i;

// loadstore output
logic       ls_reg_write;
logic[4:0]  ls_reg_addr;
logic[31:0] ls_reg_data;

// hazard output
logic       hzd_ex_discard_request;
logic       hzd_dec_stall_request;

// handshake
logic  if_dec_ready,  if_dec_valid,
       dec_ex_ready,  dec_ex_valid,  
       ex_ls_ready,   ex_ls_valid,   
       ls_valid;                        

registers registers_inst (
  .clk_i     (clk_i),

  .raddr1_i  (reg_raddr1),
  .rdata1_o  (reg_rdata1),

  .raddr2_i  (reg_raddr2),
  .rdata2_o  (reg_rdata2),

  .write_i   (reg_write),
  .waddr_i   (reg_waddr),
  .wdata_i   (reg_wdata)
);

fetch #(
 .BOOT_ADDRESS      (BOOT_ADDRESS),
 .INTERRUPT_ADDRESS (INTERRUPT_ADDRESS)
) fetch_inst (
  .clk_i            (clk_i),
  .rst_i            (rst_i),

  .irq_i            (irq_i),

  .branch_i         (branch),
  .branch_target_i  (branch_target),

  .wb_adr_o         (if_wb_adr_o),
  .wb_dat_i         (if_wb_dat_i),
  .wb_we_o          (if_wb_we_o),
  .wb_sel_o         (if_wb_sel_o),
  .wb_stb_o         (if_wb_stb_o),
  .wb_ack_i         (if_wb_ack_i),
  .wb_cyc_o         (if_wb_cyc_o),
  .wb_stall_i       (if_wb_stall_i),

  .output_ready_i   (if_dec_ready),
  .output_valid_o   (if_dec_valid),

  .instr_o          (if_instr),
  .pc_o             (if_pc)
);

decode decode_inst (
  .clk_i               (clk_i),
  .rst_i               (rst_i),

  .input_ready_o       (if_dec_ready),
  .input_valid_i       (if_dec_valid),

  .instr_i             (if_instr),
  .pc_i                (if_pc),

  .raddr1_o            (reg_raddr1),
  .rdata1_i            (reg_rdata1),
  .raddr2_o            (reg_raddr2),
  .rdata2_i            (reg_rdata2),

  .output_ready_i      (dec_ex_ready),
  .output_valid_o      (dec_ex_valid),

  .pc_o                (dec_pc),

  .alu_operand1_o      (dec_alu_operand1),
  .alu_operand2_o      (dec_alu_operand2),
  .alu_op_o            (dec_alu_op),
  .alu_sub_o           (dec_alu_sub),
  .alu_shift_left_o    (dec_alu_shift_left),
  .alu_signed_shift_o  (dec_alu_signed_shift),

  .branch_cond_o       (dec_branch_cond),
  .branch_offset_o     (dec_branch_offset),

  .reg_write_o         (dec_reg_write),
  .reg_addr_o          (dec_reg_addr),

  .ls_enable_o         (dec_ls_enable),
  .ls_write_o          (dec_ls_write),
  .ls_write_data_o     (dec_ls_write_data),
  .ls_sel_o            (dec_ls_sel),
  .ls_unsigned_load_o  (dec_ls_unsigned_load),

  .stall_request_i     (hzd_dec_stall_request)
);

execute execute_inst (
  .clk_i               (clk_i),
  .rst_i               (rst_i),

  .input_ready_o       (dec_ex_ready),
  .input_valid_i       (dec_ex_valid),

  .pc_i                (dec_pc),

  .alu_operand1_i      (dec_alu_operand1),
  .alu_operand2_i      (dec_alu_operand2),
  .alu_op_i            (dec_alu_op),
  .alu_sub_i           (dec_alu_sub),
  .alu_shift_left_i    (dec_alu_shift_left),
  .alu_signed_shift_i  (dec_alu_signed_shift),

  .ls_enable_i         (dec_ls_enable),
  .ls_write_i          (dec_ls_write),
  .ls_write_data_i     (dec_ls_write_data),
  .ls_sel_i            (dec_ls_sel),
  .ls_unsigned_load_i  (dec_ls_unsigned_load),

  .reg_write_i         (dec_reg_write),
  .reg_addr_i          (dec_reg_addr),

  .branch_cond_i       (dec_branch_cond),
  .branch_offset_i     (dec_branch_offset),

  .output_ready_i      (ex_ls_ready),
  .output_valid_o      (ex_ls_valid),

  .result_o            (ex_result),

  .ls_enable_o         (ex_ls_enable),
  .ls_write_o          (ex_ls_write),
  .ls_write_data_o     (ex_ls_write_data),
  .ls_sel_o            (ex_ls_sel),
  .ls_unsigned_load_o  (ex_ls_unsigned_load),

  .reg_write_o         (ex_reg_write),
  .reg_addr_o          (ex_reg_addr),

  .branch_o            (branch),
  .branch_target_o     (branch_target),

  .discard_request_i   (hzd_ex_discard_request)
);

loadstore loadstore_inst (
  .clk_i            (clk_i),
  .rst_i            (rst_i),

  .input_ready_o    (ex_ls_ready),
  .input_valid_i    (ex_ls_valid),

  .alu_result_i     (ex_result),
  .enable_i         (ex_ls_enable),
  .write_i          (ex_ls_write),
  .write_data_i     (ex_ls_write_data),
  .sel_i            (ex_ls_sel),
  .unsigned_load_i  (ex_ls_unsigned_load),

  .reg_write_i      (ex_reg_write),
  .reg_addr_i       (ex_reg_addr),

  .wb_adr_o         (ls_wb_adr_o),
  .wb_dat_i         (ls_wb_dat_i),
  .wb_dat_o         (ls_wb_dat_o),
  .wb_we_o          (ls_wb_we_o),
  .wb_sel_o         (ls_wb_sel_o),
  .wb_stb_o         (ls_wb_stb_o),
  .wb_ack_i         (ls_wb_ack_i),
  .wb_cyc_o         (ls_wb_cyc_o),
  .wb_stall_i       (ls_wb_stall_i),

  .output_valid_o   (ls_valid),

  .reg_write_o      (ls_reg_write),
  .reg_addr_o       (ls_reg_addr),
  .reg_data_o       (ls_reg_data)
);

writeback writeback_inst (
  .clk_i          (clk_i),
  .rst_i          (rst_i),

  .input_valid_i  (ls_valid),

  .reg_write_i    (ls_reg_write),
  .reg_addr_i     (ls_reg_addr),
  .reg_data_i     (ls_reg_data),

  .reg_write_o    (reg_write),
  .reg_addr_o     (reg_waddr),
  .reg_data_o     (reg_wdata)
);

memory memory_inst (
  .clk_i (clk_i),
  .rst_i (rst_i),

  .s1_wb_adr_i   (if_wb_adr_o),
  .s1_wb_dat_o   (if_wb_dat_i),
  .s1_wb_dat_i   ('0),
  .s1_wb_we_i    (if_wb_we_o),
  .s1_wb_sel_i   (if_wb_sel_o),
  .s1_wb_stb_i   (if_wb_stb_o),
  .s1_wb_ack_o   (if_wb_ack_i),
  .s1_wb_cyc_i   (if_wb_cyc_o),
  .s1_wb_stall_o (if_wb_stall_i),

  .s2_wb_adr_i   (ls_wb_adr_o),
  .s2_wb_dat_o   (ls_wb_dat_i),
  .s2_wb_dat_i   (ls_wb_dat_o),
  .s2_wb_we_i    (ls_wb_we_o),
  .s2_wb_sel_i   (ls_wb_sel_o),
  .s2_wb_stb_i   (ls_wb_stb_o),
  .s2_wb_ack_o   (ls_wb_ack_i),
  .s2_wb_cyc_i   (ls_wb_cyc_o),
  .s2_wb_stall_o (ls_wb_stall_i),

  .m_wb_adr_o   (wb_adr_o),
  .m_wb_dat_i   (wb_dat_i),
  .m_wb_dat_o   (wb_dat_o),
  .m_wb_we_o    (wb_we_o),
  .m_wb_sel_o   (wb_sel_o),
  .m_wb_stb_o   (wb_stb_o),
  .m_wb_ack_i   (wb_ack_i),
  .m_wb_cyc_o   (wb_cyc_o),
  .m_wb_stall_i (wb_stall_i)
);

hazard hazard_inst (
  .clk_i (clk_i),
  .rst_i (rst_i),

  .branch_i (branch),
  .ex_discard_request_o  (hzd_ex_discard_request),

  .reg_raddr1_i (reg_raddr1),
  .reg_raddr2_i (reg_raddr2),
  .dec_reg_write_i (dec_reg_write),
  .dec_reg_addr_i (dec_reg_addr),
  .ex_reg_write_i (ex_reg_write),
  .ex_reg_addr_i (ex_reg_addr),
  .ls_reg_write_i (ls_reg_write),
  .ls_reg_addr_i (ls_reg_addr),
  .reg_write_i (reg_write),
  .reg_waddr_i (reg_waddr),
  .dec_stall_request_o (hzd_dec_stall_request)
);

endmodule // ecap5_dproc
