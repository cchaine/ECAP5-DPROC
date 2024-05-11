Overall Description
===================

User needs
----------

ECAP5 is the primary user for ECAP5-DPROC. ECAP5-DPROC could however be used as a standalone RISC-V processor. The following requirements define the user needs. 

.. requirement:: U_INSTRUCTION_SET_01

   ECAP5-DPROC shall implement the RV32I instruction set.

In order to improve the usability of ECAP5-DPROC, it shall have a von Neumann architecture as it only requires one memory interface.

.. requirement:: U_MEMORY_INTERFACE_01

   ECAP5-DPROC shall access both instruction and data through a unique memory interface.


.. requirement:: U_MEMORY_INTERFACE_02

   ECAP5-DPROC's unique memory interface shall be compliant with the Wishbone specification.

.. list-table:: Wishbone Datasheet for the memory interface
  :header-rows: 1
  :width: 100%
  :widths: 70 30
  
  * - Description
    - Specification

  * - Revision level of the WISHBONE specification
    - B4
  * - Type of interface
    - MASTER
  * - Signal names for the WISHBONE interface
    - Wishbone signals are prefixed with ``wb_``.
  * - ERR_I support
    - No
  * - RTY_I support
    - No
  * - Supported tags
    - None
  * - Port size
    - 32-bit
  * - Port granularity
    - 8-bit
  * - Maximum operand size
    - 32-bit
  * - Data transfer ordering
    - Little Endian
  * - Sequence of data transfer
    - Undefined
  * - Clock constraints
    - Clocked on clk_i

.. requirement:: U_RESET_01

  ECAP5-DPROC shall provide a signal which shall hold ECAP5-DPROC in a reset state while asserted.

The polarity of the reset signal mentionned in :req:ref:`U_RESET_01` is not specified by the user.

.. requirement:: U_BOOT_ADDRESS_01

   The address at which ECAP5-DPROC jumps after the reset signal is deasserted shall be hardware-configurable.

.. requirement:: U_HARDWARE_INTERRUPT_01

  ECAP5-DPROC shall provide a signal which shall interrupt ECAP5-DPROC's execution flow while asserted.

.. requirement:: U_HARDWARE_INTERRUPT_02

   ECAP5-DPROC shall jump to a hardware-configurable address when it is interrupted.

.. requirement:: U_DEBUG_01

   ECAP5-DPROC shall be compliant with the RISC-V External Debug Support specification.

.. note:: There is no performance goal required by ECAP5 for ECAP5-DPROC as ECAP5 is an educational platform.

Assumptions and Dependencies
----------------------------

N/A
