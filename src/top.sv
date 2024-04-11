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

module top (
  input  logic        clk_i,
  input  logic        rst_i,

  input  logic        irq_i,
  input  logic        drq_i,

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

// regm
logic[4:0] reg_raddr1, reg_raddr2, reg_waddr;
logic[31:0] reg_rdata1, reg_rdata2, reg_wdata;
logic reg_write;

// branch interface
logic branch;
logic[31:0] branch_target;

// ifm wishbone
logic[31:0]  ifm_wb_adr_o;
logic[31:0]  ifm_wb_dat_i;
logic        ifm_wb_we_o;
logic[3:0]   ifm_wb_sel_o;
logic        ifm_wb_stb_o;
logic        ifm_wb_ack_i;
logic        ifm_wb_cyc_o;
logic        ifm_wb_stall_i;

// ifm output
logic[31:0] ifm_instr;
logic[31:0] ifm_pc;

// decm output
logic[31:0]  decm_pc;
logic[31:0]  decm_alu_operand1;
logic[31:0]  decm_alu_operand2;
logic[2:0]   decm_alu_op;
logic        decm_alu_sub;
logic        decm_alu_shift_left;
logic        decm_alu_signed_shift;
logic[2:0]   decm_branch_cond;
logic[19:0]  decm_branch_offset;
logic        decm_reg_write;
logic[4:0]   decm_reg_addr;
logic        decm_ls_enable;
logic        decm_ls_write;
logic[31:0]  decm_ls_write_data;
logic[3:0]   decm_ls_sel;
logic        decm_ls_unsigned_load;

// exm output
logic[31:0] exm_result;
logic exm_ls_enable;
logic exm_ls_write;
logic[31:0] exm_ls_write_data;
logic[3:0] exm_ls_sel;
logic exm_ls_unsigned_load;
logic exm_reg_write;
logic[4:0] exm_reg_addr;

// lsm wishbone
logic[31:0]  lsm_wb_adr_o;
logic[31:0]  lsm_wb_dat_i;
logic[31:0]  lsm_wb_dat_o;
logic        lsm_wb_we_o;
logic[3:0]   lsm_wb_sel_o;
logic        lsm_wb_stb_o;
logic        lsm_wb_ack_i;
logic        lsm_wb_cyc_o;
logic        lsm_wb_stall_i;

// lsm output
logic lsm_reg_write;
logic[4:0] lsm_reg_addr;
logic[31:0] lsm_reg_data;

// handshake
logic  ifm_decm_ready,  ifm_decm_valid,
       decm_exm_ready,  decm_exm_valid,  
       exm_lsm_ready,   exm_lsm_valid,   
       lsm_valid;                        

regm regm_inst (
  .clk_i     (clk_i),

  .raddr1_i  (reg_raddr1),
  .rdata1_o  (reg_rdata2),

  .raddr2_i  (reg_raddr2),
  .rdata2_o  (reg_rdata2),

  .write_i   (reg_write),
  .waddr_i   (reg_waddr),
  .wdata_i   (reg_wdata)
);

ifm ifm_inst (
  .clk_i            (clk_i),
  .rst_i            (rst_i),

  .irq_i            (irq_i),
  .drq_i            (drq_i),

  .branch_i         (branch),
  .branch_target_i  (branch_target),

  .wb_adr_o         (ifm_wb_adr_o),
  .wb_dat_i         (ifm_wb_dat_i),
  .wb_we_o          (ifm_wb_we_o),
  .wb_sel_o         (ifm_wb_sel_o),
  .wb_stb_o         (ifm_wb_stb_o),
  .wb_ack_i         (ifm_wb_ack_i),
  .wb_cyc_o         (ifm_wb_cyc_o),
  .wb_stall_i       (ifm_wb_stall_i),

  .output_ready_i   (ifm_decm_ready),
  .output_valid_o   (ifm_decm_valid),

  .instr_o          (ifm_instr),
  .pc_o             (ifm_pc)
);

decm decm_inst (
  .clk_i               (clk_i),
  .rst_i               (rst_i),

  .input_ready_o       (ifm_decm_ready),
  .input_valid_i       (ifm_decm_valid),

  .instr_i             (ifm_instr),
  .pc_i                (ifm_pc),

  .raddr1_o            (reg_raddr1),
  .rdata1_i            (reg_rdata1),
  .raddr2_o            (reg_raddr2),
  .rdata2_i            (reg_rdata2),

  .output_ready_i      (decm_exm_ready),
  .output_valid_o      (decm_exm_valid),

  .pc_o                (decm_pc),

  .alu_operand1_o      (decm_alu_operand1),
  .alu_operand2_o      (decm_alu_operand2),
  .alu_op_o            (decm_alu_op),
  .alu_sub_o           (decm_alu_sub),
  .alu_shift_left_o    (decm_alu_shift_left),
  .alu_signed_shift_o  (decm_alu_signed_shift),

  .branch_cond_o       (decm_branch_cond),
  .branch_offset_o     (decm_branch_offset),

  .reg_write_o         (decm_reg_write),
  .reg_addr_o          (decm_reg_addr),

  .ls_enable_o         (decm_ls_enable),
  .ls_write_o          (decm_ls_write),
  .ls_write_data_o     (decm_ls_write_data),
  .ls_sel_o            (decm_ls_sel),
  .ls_unsigned_load_o  (decm_ls_unsigned_load)
);

exm exm_inst (
  .clk_i               (clk_i),
  .rst_i               (rst_i),

  .input_ready_o       (decm_exm_ready),
  .input_valid_i       (decm_exm_valid),

  .pc_i                (decm_pc),

  .alu_operand1_i      (decm_alu_operand1),
  .alu_operand2_i      (decm_alu_operand2),
  .alu_op_i            (decm_alu_op),
  .alu_sub_i           (decm_alu_sub),
  .alu_shift_left_i    (decm_alu_shift_left),
  .alu_signed_shift_i  (decm_alu_signed_shift),

  .ls_enable_i         (decm_ls_enable),
  .ls_write_i          (decm_ls_write),
  .ls_write_data_i     (decm_ls_write_data),
  .ls_sel_i            (decm_ls_sel),
  .ls_unsigned_load_i  (decm_ls_unsigned_load),

  .reg_write_i         (decm_reg_write),
  .reg_addr_i          (decm_reg_addr),

  .branch_cond_i       (decm_branch_cond),
  .branch_offset_i     (decm_branch_offset),

  .output_ready_i      (exm_lsm_ready),
  .output_valid_o      (exm_lsm_valid),

  .result_o            (exm_result),

  .ls_enable_o         (exm_ls_enable),
  .ls_write_o          (exm_ls_write),
  .ls_write_data_o     (exm_ls_write_data),
  .ls_sel_o            (exm_ls_sel),
  .ls_unsigned_load_o  (exm_ls_unsigned_load),

  .reg_write_o         (exm_reg_write),
  .reg_addr_o          (exm_reg_addr),

  .branch_o            (branch),
  .branch_target_o     (branch_target)
);

lsm lsm_inst (
  .clk_i            (clk_i),
  .rst_i            (rst_i),

  .input_ready_o    (exm_lsm_ready),
  .input_valid_i    (exm_lsm_valid),

  .alu_result_i     (exm_result),
  .enable_i         (exm_ls_enable),
  .write_i          (exm_ls_write),
  .write_data_i     (exm_ls_write_data),
  .sel_i            (exm_ls_sel),
  .unsigned_load_i  (exm_ls_unsigned_load),

  .reg_write_i      (exm_reg_write),
  .reg_addr_i       (exm_reg_addr),

  .wb_adr_o         (lsm_wb_adr_o),
  .wb_dat_i         (lsm_wb_dat_i),
  .wb_dat_o         (lsm_wb_dat_o),
  .wb_we_o          (lsm_wb_we_o),
  .wb_sel_o         (lsm_wb_sel_o),
  .wb_stb_o         (lsm_wb_stb_o),
  .wb_ack_i         (lsm_wb_ack_i),
  .wb_cyc_o         (lsm_wb_cyc_o),
  .wb_stall_i       (lsm_wb_stall_i),

  .output_valid_o   (lsm_valid),

  .reg_write_o      (lsm_reg_write),
  .reg_addr_o       (lsm_reg_addr),
  .reg_data_o       (lsm_reg_data)
);

wbm wbm_inst (
  .clk_i          (clk_i),

  .input_valid_i  (lsm_valid),

  .reg_write_i    (lsm_reg_write),
  .reg_addr_i     (lsm_reg_addr),
  .reg_data_i     (lsm_reg_data),

  .reg_write_o    (reg_write),
  .reg_addr_o     (reg_waddr),
  .reg_data_o     (reg_wdata)
);

assign wb_adr_o = '0;
assign wb_dat_o = '0;
assign wb_sel_o = '0;
assign wb_we_o = 0;
assign wb_stb_o = 0;
assign wb_cyc_o = 0;

assign ifm_wb_dat_i = 0;
assign ifm_wb_ack_i = 0;
assign ifm_wb_stall_i = 1;

assign lsm_wb_dat_i = 0;
assign lsm_wb_ack_i = 0;
assign lsm_wb_stall_i = 1;

endmodule // top
