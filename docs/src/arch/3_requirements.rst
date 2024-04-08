Requirements
============

External Interface Requirements
-------------------------------

.. list-table:: ECAP5-DPROC control signals
  :header-rows: 1
  :width: 100%
  :widths: 10 10 10 70

  * - Name
    - Type
    - Width
    - Description

  * - clk_i
    - I
    - 1
    - Clock input.
  * - rst_i
    - I
    - 1
    - Hardware reset.
  * - irq_i
    - I
    - 1
    - External interrupt request.
  * - drq_i
    - I
    - 1
    - Debug request.

.. requirement:: I_CLK_01

   All inputs and outputs of ECAP5-DPROC shall belong to clk_i's clock domain.

.. requirement:: I_RESET_01
   :derivedfrom: U_RESET_01

   The rst i signal shall hold ECAP5-DPROC in a reset state while asserted.

.. requirement:: I_IRQ_01
   :derivedfrom: U_HARDWARE_INTERRUPT_01

    ECAP5-DPROC shall interrupt its execution flow when input irq_i is asserted.

.. requirement:: I_DRQ_01
   :derivedfrom: U_DEBUG_01

    ECAP5-DPROC shall interrupt its execution flow when input drq_i is asserted.

.. list-table:: ECAP5-DPROC memory interface signals
  :header-rows: 1
  :width: 100%
  :widths: 10 10 10 70

  * - Name
    - Type
    - Width
    - Description

  * - wb_clk_i
    - I
    - 1
    - The clock input coordinates all activites for the internal logic within the wishbone interconnect. All wishbone output signals are registered at the rising edge of wb_clk_i. All wishbone input signals are stable before the rising edge of wb_clk_i.
  * - wb_rst_i
    - I
    - 1
    - The reset input forces the wishbone interface to restart. Furthermore, all internal wishbone self-starting state machines will be forced into an initial state. This signal only resets the wishbone interface.
  * - wb_adr_o
    - O
    - 32
    - The address output array is used to pass binary address.
  * - wb_dat_i
    - I
    - 32
    - The data input array is used to pass binary data.
  * - wb_dat_o
    - O
    - 32
    - The data output array is used to pass binary data.
  * - wb_sel_o
    - O
    - 4
    - The select output array indicates where valid data is expected on the wb_dat_i signal array during READ cycles, and where it is placed on the wb_dat_o signal array during WRITE cycles. Each individual select signal correlates to one of four active bytes on the 32-bit data port.
  * - wb_we_o
    - O
    - 1
    - The write enable output indicates whether the current local bus cycle is a READ or WRITE cycle. This signal is negated during READ cycles and is asserted during WRITE cycles.
  * - wb_stb_o
    - O
    - 1
    - The strobe output indicates a valid data transfer cycle. It is used to qualify various other signals on the interface.
  * - wb_ack_i
    - I
    - 1
    - The acknowledge input, when asserted, indicates the normal termination of a bus cycle.
  * - wb_cyc_o
    - O
    - 1
    - The cycle output, when asserted, indicates that a valid bus cycle is in progress. This signal is asserted for the duration of all bus cycles.
  * - wb_stall_i
    - O
    - 1
    - The pipeline stall input indicates that current slave is not able to accept the transfer in the transaction queue.

.. requirement:: I_MEMORY_INTERFACE_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  Signals from table 3 shall be compliant with the Wishbone specification.

Functional Requirements
-----------------------

.. requirement:: F_IRQ_HANDLER_01
   :derivedfrom: U_HARDWARE_INTERRUPT_02

   ECAP5-DPROC shall jump to a hardware-configurable address when irq_i is asserted.

.. requirement:: F_DRQ_HANDLER_01
   :derivedfrom: U_DEBUG_01
  
   ECAP5-DPROC shall jump to a hardware-configurable address when drq_i is asserted.

Register file
^^^^^^^^^^^^^

.. requirement:: F_REGISTER_01
   :derivedfrom: U_INSTRUCTION_SET_01
  
   ECAP5-DPROC shall implement 32 user-accessible general purpose regis- ters ranging from x0 to x31.

.. requirement:: F_REGISTER_02
   :derivedfrom: U_INSTRUCTION_SET_01

   Register x0 shall always be equal to zero.

.. requirement:: F_REGISTER_03
   :derivedfrom: U_INSTRUCTION_SET_01

   ECAP5-DPROC shall implement a pc register storing the address of the current instruction.

.. requirement:: F_REGISTER_RESET_01
   :derivedfrom: U_BOOT_ADDRESS_01

   The pc register shall be loaded with an hardware-configurable address when ECAP5-DPROC leaves its reset state.

Instruction decoding
^^^^^^^^^^^^^^^^^^^^

Figure 2 outlines the different instruction encodings for the RV32I instruction set. The opcode parameter is a unique identifier for each instruction. The instruction encoding is infered from the opcode as there can only be one encoding per opcode.

.. todo:: Add instruction encoding diagram

Immediate encoding
^^^^^^^^^^^^^^^^^^

Only one immediate value can be encoded in one instruction. The value can be re- constructed from fragments of the following format : imm[x] representing the x:sup:th bit or imm[x:y] representing bits from the xth to the yth both included.

.. requirement:: F_INSTR_IMMEDIATE_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Immediate values shall be sign-extended.

.. requirement:: F_INSTR_IMMEDIATE_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The value of an instruction immediate shall be the concatenation of immediate fragments from the instruction encoding.

.. requirement:: F_INSTR_IMMEDIATE_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Missing immediate fragments shall be replaced by zeros.

RV32I is called a Load/Store ISA, meaning that instructions inputs and outputs are passed through registers or through an instruction immediate. There are specific instructions for loading and storing data into memory.

Instruction parameters
^^^^^^^^^^^^^^^^^^^^^^

.. requirement:: F_INSTR_FIRST_PARAM_01
  :derivedfrom: U_INSTRUCTION_SET_01

   Instructions encoded using the R-type, I-type, S-type and B-type shall take as their first parameter the value stored in the register designated by the rs1 field.

.. requirement:: F_INSTR_FIRST_PARAM_02
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the U-type and J-type shall take as their first parameter the immediate value encoded in the instruction.

.. requirement:: F_INSTR_SECOND_PARAM_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the R-type, S-type and B-type shall take as their second parameter the value stored in the register designated by the rs2 field.

.. requirement:: F_INSTR_SECOND_PARAM_02
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the I-type shall take as its second parameter the immediate value encoded in the instruction.

.. requirement:: F_INSTR_THIRD_PARAM_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the S-type and B-type shall take as their third parameter the immediate value encoded in the instruction.

Instruction results
^^^^^^^^^^^^^^^^^^^

.. requirement:: F_INSTR_VARIANT_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the R-type, I-type, S-type and B-type shall use the func3 field as a behavior variant selector.

.. requirement:: F_INSTR_VARIANT_02
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the R-type shall use the func7 field as a secondary behavior variant selector.

.. requirement:: F_INSTR_VARIANT_03
  :rationale: The SRLI and SRAI instructions use the I-type encoding but only the 5 LSBs of the immediate parameter are used for the behavior. The other 7 MSBs are assimilated to the func7 field of the R-type encoding.
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions encoded using the OP-IMM opcode and either SRLI-FUNC3 or SRAI-FUNC3 shall use the 7 MSBs of their second parameter as a secondary behavior variant selector.

Opcodes
^^^^^^^

Table 4 outlines the different opcodes values of the RV32I instruction set. Cells marked as noimp are for opcodes that are not implemented in ECAP5-DPROC.

.. todo:: Add opcode value table

.. requirement:: F_OPCODE_ENCODING_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the LUI opcode shall be decoded as an U-type in- struction.

.. requirement:: F_OPCODE_ENCODING_02
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the AUIPC opcode shall be decoded as an U-type instruction.

.. requirement:: F_OPCODE_ENCODING_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the JAL opcode shall be decoded as a J-type instruc- tion.

.. requirement:: F_OPCODE_ENCODING_04
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the JALR opcode shall be decoded as an I-type in- struction.

.. requirement:: F_OPCODE_ENCODING_05
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the BRANCH opcode shall be decoded as a B-type instruction.

.. requirement:: F_OPCODE_ENCODING_06
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the LOAD opcode shall be decoded as an I-type in- struction.

.. requirement:: F_OPCODE_ENCODING_07
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the STORE opcode shall be decoded as a S-type instruction.

.. requirement:: F_OPCODE_ENCODING_08
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the OP-IMM opcode shall be decoded as an I-type instruction.

.. requirement:: F_OPCODE_ENCODING_09
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the OP opcode shall be decoded as a R-type instruction.

.. requirement:: F_OPCODE_ENCODING_10
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the MISC-MEM opcode shall be decoded as an I-type instruction.

.. requirement:: F_OPCODE_ENCODING_11
  :derivedfrom: U_INSTRUCTION_SET_01

  Instructions which use the SYSTEM opcode shall be decoded as an I-type instruction.

Instruction behaviors
^^^^^^^^^^^^^^^^^^^^^

LUI
```

.. requirement:: F_LUI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LUI behavior shall be applied when the opcode is LUI.

.. requirement:: F_LUI_02
  :rationale: The LUI instruction shall load the 20 upper bits of the instruction immediate into the destination register and fill the remaining bits with zeros. This is the default behavior for instruction immediates as stated in F_INSTR_IMMEDIATE_02 and F_INSTR_IMMEDIATE_03.
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LUI shall be the value of its first parameter.

AUIPC
`````

.. requirement:: F_AUIPC_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The AUIPC behavior shall be applied when the opcode is AUIPC.

.. requirement:: F_AUIPC_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of AUIPC shall be the sum of its first parameter and the address of the AUIPC instruction.

JAL
```

.. requirement:: F_JAL_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The JAL behavior shall be applied when the opcode is JAL.

.. requirement:: F_JAL_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The pc register shall be updated with the sum of the address of the JAL instruction with the first instruction parameter.

.. requirement:: F_JAL_03
  :rationale: The JAL instruction shall output the address to the following instruction for it to be used as a *return address* in the case of a function call.
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of JAL shall be the address of the JAL instruction incremented by 4.

JALR
````

.. requirement:: F_JALR_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The JALR behavior shall be applied when the opcode is JALR and func3 is 0x0.

.. requirement:: F_JALR_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The pc register shall be updated with the sum of the first and second param- eters of the JALR instruction.

.. requirement:: F_JALR_03
  :rationale: The JALR instruction shall output the address to the following instruction for it to be used as a *return address* in the case of a function call.
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of JALR shall be the address of the JALR instruction incremented by 4.

BEQ
```

.. requirement:: F_BEQ_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BEQ behavior shall be applied when the opcode is BRANCH and func3 is 0x0.

.. requirement:: F_BEQ_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first and second instruction parameters are equal, the pc register shall be updated with the signed sum of the address of the BEQ instruction with the third parameter.

BNE
```

.. requirement:: F_BNE_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BNE behavior shall be applied when the opcode is BRANCH and func3 is 0x1.

.. requirement:: F_BNE_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first and second parameters are not equal, the pc register shall be updated with the signed sum of the address of the BNE instruction with the third parameter.

BLT
```

.. requirement:: F_BLT_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BLT behavior shall be applied when the opcode is BRANCH and func3 is 0x4.

.. requirement:: F_BLT_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first parameter is lower than the second parameter using a signed comparison, the pc register shall be updated with the signed sum of the address of the BLT instruction with the third parameter.

BGE
```

.. requirement:: F_BGE_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BGE behavior shall be applied when the opcode is BRANCH and func3 is 0x5.

.. requirement:: F_BGE_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first parameter is greater or equal to the second parameter using a signed comparison, the pc register shall be updated with the signed sum of the address of the BGE instruction with the third parameter.

BLTU
````

.. requirement:: F_BLTU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BLTU behavior shall be applied when the opcode is BRANCH and func3 is 0x6.

.. requirement:: F_BLTU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first parameter is lower than the second parameter using an unsigned comparison, the pc register shall be updated with the signed sum of the address of the BLTU instruction with the third parameter.

BGEU
````

.. requirement:: F_BGEU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The BGEU behavior shall be applied when the opcode is BRANCH and func3 is 0x7.

.. requirement:: F_BGEU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  When the first parameter is greater or equal to the second parameter using an unsigned comparison, the pc register shall be updated with the signed sum of the address of the BGEU instruction with the third parameter.

LB
``

.. requirement:: F_LB_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LB behavior shall be applied when the opcode is LOAD and func3 is 0x0.

.. requirement:: F_LB_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LB shall be the 8-bit value stored in memory at the address determined by the signed sum of its first and second parameters.

.. requirement:: F_LB_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The remaining bits of the loaded value shall be filled with the value of its 7th bit.

LH
``

.. requirement:: F_LH_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LH behavior shall be applied when the opcode is LOAD and func3 is 0x1.

.. requirement:: F_LH_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LH shall be the 16-bit value stored in memory at the address determined by the signed sum of its first and second parameters.

.. requirement:: F_LH_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The remaining bits of the loaded value shall be filled with the value of its 15th bit.

LW
``

.. requirement:: F_LW_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LW behavior shall be applied when the opcode is LOAD and func3 is 0x2.

.. requirement:: F_LW_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LW shall be the 32-bit value stored in memory at the address determined by the signed sum of its first and second parameters.

LBU
```

.. requirement:: F_LBU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LBU behavior shall be applied when the opcode is LOAD and func3 is 0x4.

.. requirement:: F_LBU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LBU shall be the 8-bit value stored in memory at the address determined by the signed sum of its first and second parameters.

.. requirement:: F_LBU_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The remaining bits of the loaded value shall be filled with zeros.

LHU
```

.. requirement:: F_LHU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The LHU behavior shall be applied when the opcode is LOAD and func3 is 0x5.

.. requirement:: F_LHU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of LHU shall be the 16-bit value stored in memory at the address determined by the signed sum of its first and second parameters.

.. requirement:: F_LHU_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The remaining bits of the loaded value shall be filled with zeros.

SB
``

.. requirement:: F_SB_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SB behavior shall be applied when the opcode is STORE and func3 is 0x0.

.. requirement:: F_SB_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The lowest byte of the second parameter of SB shall be stored in memory at the address determined by the signed sum of its first and third parameters.

SH
``

.. requirement:: F_SH_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SH behavior shall be applied when the opcode is STORE and func3 is 0x1.

.. requirement:: F_SH_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The two lowest bytes of the second parameter of SB shall be stored in memory at the address determined by the signed sum of its first and third param- eters.

SW
``

.. requirement:: F_SW_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SW behavior shall be applied when the opcode is STORE and func3 is 0x2.

.. requirement:: F_SW_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The value of the second parameter of SB shall be stored in memory at the address determined by the signed sum of its first and third parameters.

ADDI
````

.. requirement:: F_ADDI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The ADDI behavior shall be applied when the opcode is OP-IMM and when func3 is 0x0.

.. requirement:: F_ADDI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ADDI shall be the signed integer sum of its two parameters.

.. requirement:: F_ADDI_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ADDI shall be truncated to 32-bits.

SLTI
````

.. requirement:: F_SLTI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLTI behavior shall be applied when the opcode is OP-IMM and when func3 is 0x2.

.. requirement:: F_SLTI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLTI shall be 1 when the signed value of its first parameter is lower that the signed value of its second parameter. It shall be 0 otherwise.

SLTIU
`````

.. requirement:: F_SLTIU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLTIU behavior shall be applied when the opcode is OP-IMM and when func3 is 0x3.

.. requirement:: F_SLTIU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLTI shall be 1 when the unsigned value of its first parameter is lower that the unsigned value of its second parameter. It shall be 0 otherwise.

XORI
````

.. requirement:: F_XORI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The XORI behavior shall be applied when the opcode is OP-IMM and when func3 is 0x4.

.. requirement:: F_XORI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of XORI shall be the result of a bitwise xor between its two pa- rameters.

ORI
```

.. requirement:: F_ORI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The ORI behavior shall be applied when the opcode is OP-IMM and when func3 is 0x6.

.. requirement:: F_ORI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ORI shall be the result of a bitwise or between its two parameters.

ANDI
````

.. requirement:: F_ANDI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The ANDI behavior shall be applied when the opcode is OP-IMM and when func3 is 0x7.

.. requirement:: F_ANDI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ANDI shall be the result of a bitwise and between its two parameters.

SLLI
````

.. requirement:: F_SLLI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLLI behavior shall be applied when the opcode is OP-IMM and func3 is 0x1.

.. requirement:: F_SLLI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLLI shall be its first parameter shifted left by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SLLI_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Zeros shall be inserted in the lower bits when shifting.

SRLI
````

.. requirement:: F_SRLI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SRLI behavior shall be applied when the opcode is OP-IMM, func3 is 0x5 and the 30th bit of its second input is 0.

.. requirement:: F_SRLI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SRLI shall be its first parameter shifted right by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SRLI_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Zeros shall be inserted in the upper bits when shifting.

SRAI
````

.. requirement:: F_SRAI_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SRAI behavior shall be applied when the opcode is OP-IMM, func3 is 0x5 and the 30th bit of its second input is 1.

.. requirement:: F_SRAI_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SRAI shall be its first parameter shifted right by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SRAI_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The most significant bit of the first parameter shall be inserted in the upper bits when shifting.

ADD
```

.. requirement:: F_ADD_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The ADD behavior shall be applied when the opcode is OP, func3 is 0x0 and func7 is 0x0.

.. requirement:: F_ADD_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ADD shall be the signed integer sum of its two parameters.

.. requirement:: F_ADD_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of ADD shall be truncated to 32-bits.

SUB
```

.. requirement:: F_SUB_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SUB behavior shall be applied when the opcode is OP, func3 is 0x0 and func7 is 0x20.

.. requirement:: F_SUB_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SUB shall be the signed integer difference of its first parameter minus its second parameter.

.. requirement:: F_SUB_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SUB shall be truncated to 32-bits.

SLL
```

.. requirement:: F_SLL_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLL behavior shall be applied when the opcode is OP and func3 is 0x1.

.. requirement:: F_SLL_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLL shall be its first parameter shifted left by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SLL_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Zeros shall be inserted in the lower bits when shifting.

SLT
```

.. requirement:: F_SLT_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLT behavior shall be applied when the opcode is OP and func3 is 0x2.

.. requirement:: F_SLT_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLT shall be 1 when the signed value of its first parameter is lower that the signed value of its second parameter. It shall be 0 otherwise.

SLTU
````

.. requirement:: F_SLTU_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SLTU behavior shall be applied when the opcode is OP and func3 is 0x3.

.. requirement:: F_SLTU_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SLTU shall be 1 when the unsigned value of its first parameter is lower that the unsigned value of its second parameter. It shall be 0 other- wise.

XOR
```

.. requirement:: F_XOR_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The XOR behavior shall be applied when the opcode is OP and func3 is 0x4.

.. requirement:: F_XOR_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of XOR shall be the result of a bitwise xor between its two parameters.

SRL
```

.. requirement:: F_SRL_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SRL behavior shall be applied when the opcode is OP, func3 is 0x5 and func7 is 0x0.

.. requirement:: F_SRL_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SRL shall be its first parameter shifted right by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SRL_03
  :derivedfrom: U_INSTRUCTION_SET_01

  Zeros shall be inserted in the upper bits when shifting.

SRA
```

.. requirement:: F_SRA_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The SRA behavior shall be applied when the opcode is OP, func3 is 0x5 and func7 is 0x20.

.. requirement:: F_SRA_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of SRA shall be its first parameter shifted right by the amount specified by the first 5 bits of its second parameter.

.. requirement:: F_SRA_03
  :derivedfrom: U_INSTRUCTION_SET_01

  The most significant bit of the first parameter shall be inserted in the upper bits when shifting.

OR
``

.. requirement:: F_OR_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The OR behavior shall be applied when the opcode is OP and func3 is 0x6.

.. requirement:: F_OR_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of OR shall be the result of a bitwise or between its two parame- ters.

AND
```

.. requirement:: F_AND_01
  :derivedfrom: U_INSTRUCTION_SET_01

  The AND behavior shall be applied when the opcode is OP and func3 is 0x7.

.. requirement:: F_AND_02
  :derivedfrom: U_INSTRUCTION_SET_01

  The result of AND shall be the result of a bitwise and between its two parameters.

FENCE
`````

.. todo:: Add requirements for the FENCE instruction

ECALL
`````

.. todo:: Add requirements for the ECALL instruction 

EBREAK
``````

.. todo:: Add requirements for the EBREAK instruction

Exceptions
^^^^^^^^^^

.. requirement:: F_INSTR_ADDR_MISALIGNED_01
  :derivedfrom: U_INSTRUCTION_SET_01

  An Instruction Address Misaligned exception shall be raised when the target address of a taken branch or an unconditional jump if not four-byte aligned.

.. requirement:: F_MISALIGNED_MEMORY_ACCESS_01
  :derivedfrom: U_INSTRUCTION_SET_01

  A Misaligned Memory Access exception shall be raised when the target address of a load/store instruction is not aligned on the referenced type size.

Memory interface
^^^^^^^^^^^^^^^^

Memory accesses
```````````````

.. requirement:: F_MEMORY_INTERFACE_01
  :derivedfrom: U_INSTRUCTION_SET_01

  Both instruction and data accesses shall be handled by a unique external memory interface.

Wishbone protocol
`````````````````

The following requirements are extracted from the Wishbone specification.

.. requirement:: F_WISHBONE_DATASHEET_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall comply with the Wishbone Datasheet provided in section 2.1.

.. requirement:: F_WISHBONE_RESET_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall initialize itself at the rising edge of wb_clk_i following the assertion of wb_rst_i.

.. requirement:: F_WISHBONE_RESET_02
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall stay in the initialization state until the rising edge of wb_clk_i following the deassertion of wb_rst_i.

.. requirement:: F_WISHBONE_RESET_03
  :derivedfrom: U_MEMORY_INTERFACE_02

  Signals wb_stb_o and wb_cyc_o shall be deasserted while the memory interface is in the initialization state. The state of all other memory interface signals are undefined in response to a reset cycle.

.. requirement:: F_WISHBONE_TRANSFER_CYCLE_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall assert wb_cyc_o for the entire duration of the memory access.

.. requirement:: F_WISHBONE_TRANSFER_CYCLE_02
  :derivedfrom: U_MEMORY_INTERFACE_02

  Signal wb_cyc_o shall be asserted no later than the rising edge of wb_clk_i that qualifies the assertion of wb_stb_o.

.. requirement:: F_WISHBONE_TRANSFER_CYCLE_03
  :derivedfrom: U_MEMORY_INTERFACE_02

  Signal wb_cyc_o shall be deasserted no earlier than the rising edge of wb_clk_i that qualifies the deassertion of wb_stb_o.

.. requirement:: F_WISHBONE_HANDSHAKE_02
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface must qualify the following signals with wb_stb_o : wb_adr_o, wb_dat_o, wb_sel_o and wb_we_o.

.. requirement:: F_WISHBONE_STALL_01
  :rationale: wb_stall_i is asserted to indicate that the request queue is temporary full and the request shall be resent.
  :derivedfrom: U_MEMORY_INTERFACE_02

  While initiating a request, the memory interface shall hold the state of its outputs until wb_stall_i is deasserted.

.. todo:: Add timing diagram for the single read cycle

.. todo:: Add description table of the single read cycle

.. requirement:: F_WISHBONE_READ_CYCLE_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  A read transaction shall be started by asserting both wb_cyc_o and wb_stb_i, and deasserting wb_we_o.

.. requirement:: F_WISHBONE_READ_CYCLE_02
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall conform to the READ cycle detailed in figure 3.

.. todo:: Add timing diagram for the single write cycle

.. todo:: Add description table of the single write cycle

.. requirement:: F_WISHBONE_WRITE_CYCLE_01
  :derivedfrom: U_MEMORY_INTERFACE_02

  A write transaction shall be started by asserting wb_cyc_o, wb stb i and wb_we_o.

.. requirement:: F_WISHBONE_WRITE_CYCLE_02
  :derivedfrom: U_MEMORY_INTERFACE_02

  The memory interface shall conform to the WRITE cycle detailed in figure 4.

.. requirement:: F_WISHBONE_TIMING_01
  :rationale: As long as the memory interface is designed within the clock domain of clk_i, the requirement will be satisfied by using the place and route tool.
  :derivedfrom: U_MEMORY_INTERFACE_02

  The clock input clk i shall coordinate all activites for the internal logic within the memory interface. All output signals of the memory interface shall be registered at the rising edge of clk_i. All input signals of the memory interface shall be stable before the rising edge of clk_i.

BLOCK cycles are not supported in revision 1.0.0

Caches
``````

Caches are not supported in revision 1.0.0

Debugging
^^^^^^^^^

.. todo:: Add requirements from the riscv debug


Non-functional Requirements
---------------------------

N/A
