OUTPUT_ARCH( "riscv" )
ENTRY(_start)

SECTIONS
{
  . = 0x1000;
  .text : { *(.text) }
  . = ALIGN(4);
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  _end = .;

  __OUTPUT__ = 0x80000000;
  __END__ = 0xA0000000;
}

