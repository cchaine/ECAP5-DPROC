Architecture Overview
=====================

.. image:: ../assets/architecture.svg

Clock domains
-------------

To simplify the design of version 1.0.0, each module of ECAP5-DPROC belong to a unique clock domain.

Pipeline stages
---------------

ECAP5-DPROC is built around a pipelined architecture with the following stages :
 * The instruction fetch stage loads the next instruction from memory.
 * The decode stage handles the instruction decoding to provide the next stage with the different instruction input values including reading from internal registers.
 * The execute stage implements all arithmetic and logic operations.
 * The load/store stage implements load/store operations.
 * The write-back stage which handles storing instructions outputs to internal registers.

Pipeline stall
^^^^^^^^^^^^^^

In order to handle pipeline stalls, a handshaking mechanism is implemented between each stages, allowing the execution flow to be stopped. A stall can be either triggered by a stage itself or requested by the hazard module.

.. todo:: Add pipeline state diagram

Pipeline stages located at the start and end of the pipeline do not implement the bubble and wait modes respectively.

The following points describe the behavior of the different modes :
 * A stage in normal mode shall operate as described by its different functional behaviors.
 * A stage in stall mode shall deassert its input ready signal and output valid signal while waiting to unstall.
 * A stage in bubble mode shall operate as normal but taking a nop instruction as input instead of the data provided by the preceding stage.
 * A stage in wait mode shall deassert its input ready signal and wait until going back to normal mode.

In case of a stall, the stalling stage deasserts its input ready signal leading to preced- ing stages waiting for completion. The stalling stage deasserts its output valid signal leading to following stages taking a bubble as their input.

The figure 7 is a diagram of the stall behavior on a 5-stage pipeline. By stalling the 3rd stage, this example provides a representative visualisation of all the stalling cases of a 4-stage pipeline.

.. todo:: Add pipeline stall diagram

 Figure 8 outlines the resolution of a pipeline stall on stage 3. By stalling the 3rd stage, this example provides a representative visualisation of all the stalling cases of a 4- stage pipeline.

.. todo:: Add pipeline stall timing-diagram

Hazard management
-----------------

Structural hazard
^^^^^^^^^^^^^^^^^

For the scope of this document, are designated as structural hazards all cases when a stage is unable to finish its processing within the required time before the next clock cycle.

A pipeline stall is produced in case of structural hazards.

.. note:: It shall be noted that the some of the performance impact of this kind of hazard could be mitigated but this feature is not included in version 1.0.0.

Data hazard
^^^^^^^^^^^

A data hazard occurs when an instruction (A) uses the result of a previous instruction (B) which is still being processed in the pipeline.

A pipeline stall is produced in case of data hazards so that B is able to finish before A uses its result.

.. note:: It shall be noted that some of the performance impact of this kind of hazard could be mitigated but this feature is not included in version 1.0.0.

Control hazard
^^^^^^^^^^^^^^

A control hazard occurs when a jump or branch instruction is executed, as instructions following the jump/branch are already being processes through the pipeline when the jump/branch happens.

Instructions following the jump/branch are replaced by a nop instruction through the use of the bubble mode of the pipeline stages. This operation is designated as bubble drop.

.. note:: It shall be noted that some of the performance impact of this kind of hazard could be mitigated but this feature is not included in version 1.0.0.

Architecture specification
--------------------------

Functional partitioning
^^^^^^^^^^^^^^^^^^^^^^^

.. requirement:: A_FUNCTIONAL_PARTITIONING_01

  The Memory module shall arbitrate memory requests from both the fetch module and the loadstore module.

.. requirement:: A_FUNCTIONAL_PARTITIONING_02
  
  The Fetch module shall implement the instruction fetch stage of the pipeline.

.. requirement:: A_INSTRUCTION_FETCH_01
  :rationale: Pipeline stages are all run in parallel, refer to section 5.2.

  The fetch module shall fetch instructions continuously starting on the clock cycle after rst i is deasserted, providing them to the decode module one after the other.

.. requirement:: A_FUNCTIONAL_PARTITIONING_03

  The Decode module shall implement the decode stage of the pipeline.

.. requirement:: A_FUNCTIONAL_PARTITIONING_04

   The Register module shall implement the internal general-purpose registers.

.. requirement:: A_FUNCTIONAL_PARTITIONING_05

   The Execute module shall implement the execute stage of the pipeline.

.. requirement:: A_FUNCTIONAL_PARTITIONING_06

   The Loadstore module shall implement the load/store stage of the pipeline.

.. requirement:: A_FUNCTIONAL_PARTITIONING_07

   The Writeback module shall implement the write-back stage of the pipeline.

.. requirement:: A_FUNCTIONAL_PARTITIONING_08

  The Hazard module shall handle the detection of data and control hazards as well as trigger the associated pipeline stalls and bubble drops.

Interface protocols
^^^^^^^^^^^^^^^^^^^

.. requirement:: A_MEMORY_BUS_01

   The bus interface between the fetch module and memory module shall be compliant with the pipelined wishbone B4 specification.

.. requirement:: A_MEMORY_BUS_02

   The bus interface between the loadstore module and memory module shall be compliant with the pipelined wishbone B4 specification.
