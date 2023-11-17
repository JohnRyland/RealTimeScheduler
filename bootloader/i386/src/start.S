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
  #movl   $copyright_msg,%esi
  #movb   $0x07,%ah
  #call   print32

  movl   $0x12345678,%eax
  #movl   $0xdeadbeef,%ebx
  #movl   $0xcafe4321,%ecx
  movl   $0x8765feed,%edx
#  call   panic32

  call   _start32

welcome_msg:
  .asciz "MagicCore RTOS v1.0"

#copyright_msg:
#  .asciz "Copyright (C) 2023, John Ryland"

line_no:
  .long 800    # Text position

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


kernel_panic_msg:
  .asciz "\n\nKernel Panic!\n"

hex_digits:
  .ascii "0123456789ABCDEF-"

kernel_panic_registers_msg:
  .ascii "\nRegisters:\n\n"
  .ascii "Flg: 0x00000000"
start_dump:
  .ascii "\n"
  .ascii "EAX: 0x00000000 EBX: 0x00000000 ECX: 0x00000000 EDX: 0x00000000\n"
  .ascii "EBP: 0x00000000 ESP: 0x00000000 ESI: 0x00000000 EDI: 0x00000000\n"
  .ascii "DS:  0x00000000 ES:  0x00000000 FS:  0x00000000 GS:  0x00000000\n"
  .ascii "CS:  0x00000000 EIP: 0x00000000\n"
  .ascii "\n"
  .asciz "Call trace: "

panic32:
  pushl  %cs
  pushl  %gs
  pushl  %fs
  pushl  %es
  pushl  %ds
  pushl  %edi
  pushl  %esi
  pushl  %esp
  pushl  %ebp
  pushl  %edx
  pushl  %ecx
  pushl  %ebx
  pushl  %eax
  pushfl

  cli
  movl   $kernel_panic_msg,%esi
  movb   $0x0C,%ah
  call   print32

  movl   $kernel_panic_registers_msg,%esi
  mov    $15,%ecx
  mov    $start_dump,%edi
next_reg:
  mov    $15,%edx
  sub    %ecx,%edx
  movl   %ss:-68(%ebp,%edx,4),%eax
  mov    %edi,%edx
next_hex_char:
  movl   %eax,%ebx
  andl   $0xF,%ebx
  movb   hex_digits(%ebx),%bl
  decl   %edi
  movb   %bl,(%edi)
  shrl   $4,%eax
  jnz    next_hex_char
  mov    %edx,%edi
  add    $16,%edi
  loop   next_reg

  movl   $kernel_panic_registers_msg,%esi
  movb   $0x0E,%ah
  call   print32
halt32:
  hlt
  jmp    halt32


# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
.org 0x7E00 + 510
boot_signature:
  .word 0xAA55


# Our own magic value for checking the second stage loaded properly
.org 0x7E00+1024-2
  .word 0x55AA
