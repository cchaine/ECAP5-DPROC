.extern __END__    # Address to jump to at the end
.extern __OUTPUT__ # Address used to write a string output

  .section .rodata
helloworld:      .ascii "Hello World!\n\0"

  .section .text
  .globl _start
_start:
  la s1, helloworld # char pointer
loop:
  lb a0, 0(s1)      # load the current char
  beq a0, x0, end   # check if null char
  call putc         # send the loaded char
  addi s1, s1, 1    # increment the char pointer
  jal x0, loop      # loop
end:
  la t0, __END__    # load the end pointer
  jalr x0, t0, 0    # jump to the end address
  
putc:
  la t0, __OUTPUT__   # load the output address
  sb a0, 0(t0)        # store the provided value
  ret
