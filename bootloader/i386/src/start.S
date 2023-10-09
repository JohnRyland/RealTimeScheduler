#
# x86 OS Kernel Loader
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

.code32

.org 0x7E00

.globl start
start:
  movl   $welcome_msg,%esi
  movb   $0x0A,%ah
  call   print32
  movl   $copyright_msg,%esi
  movb   $0x07,%ah
  call   print32
  call   _start32

welcome_msg:
  .asciz "MagicCore RTOS v1.0"

copyright_msg:
  .asciz "Copyright (C) 2023, John Ryland"

line_no:
  .long 1120    # Text position

# Prints null terminated string %si direct to VGA memory
# First byte pointed to is the text attribute byte to use
print32:
  movl   $0xb8000,%edi    # text video memory address
  addl   (line_no),%edi
 print_loop32:
  lodsb
  cmp    $'\n',%al
  je     crlf
  stosw
  or     %al,%al
  jne    print_loop32
new_line:
  movl   (line_no),%edi
  addl   $160,%edi
  movl   %edi,(line_no)   # move to the next line
  # Note: doesn't handle scrolling the screen so don't print more than 24 lines
  ret
crlf:
  call   new_line
  jmp    print32

.globl _k_panic
_k_panic:
  pushfl
  cli
  pushal
  pushl  %ds
  pushl  %es
  pushl  %fs
  pushl  %gs
  pushl  %cs
  call   _panic_helper
  ret

# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
.org 0x7EB0
boot_signature:
  .word 0x55AA
