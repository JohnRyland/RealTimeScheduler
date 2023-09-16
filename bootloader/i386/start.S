# CPU starts up in 16-bit mode
.code16

# The code is loaded by the BIOS at 0x7C00 so make our addresses match up
.org 0x7C00

# Setup the registers and then jump to main
_start:
  push   %cs
  pop    %ds
  movw   $0x3,%ax
  int    $0x10     # reset the screen mode
  jmp    main

# Prints null terminated string %si direct to VGA memory
# First byte pointed to is the text attribute byte to use
print:
  push   $0xb800  # text video memory address
  pop    %es
  movw   (line_no),%di
  lodsb
  mov    %al,%ah
 print_loop:
  lodsb
  stosw
  cmp    $0,%al
  jne    print_loop
  movw   (line_no),%di
  addw   $160,%di
  movw   %di,(line_no)   # move to the next line
  # TODO: doesn't handle scrolling the screen if fill the screen
  ret

# Panic and stop running
panic:
  movw   $halt_msg,%si
  call   print
 halt:
  hlt
  jmp    halt

# Enable A20 Gate
enable_a20:
  movw   $0x2401,%ax
  int    $0x15
  ret

# Setup 32bit mode
setup_32:
  # Load Global Descriptor Table
  lgdt   gdt32ptr

  # Load Control Register CR0
  # Enable Protected Mode (Bit0 in CR0)
  # Store Control Register CR0
  movl   %cr0,%eax
  orl    $(1<<0),%eax
  movl   %eax,%cr0

  # Setup all Segment Registers
  # and reload Code Segment, Instruction Pointer
  xorl   %eax,%eax
  movw   $0x10,%ax
  movl   %eax,%ds
  movl   %eax,%es
  movl   %eax,%fs
  movl   %eax,%gs
  ljmp   $0x0008,$(enter_protected_mode)


# Read the second stage boot loader after the master boot record
read_second_stage:
  movw   $lba_data,%si
  movb   $0x42,%ah
  int    $0x13
  jc     error
  movw   $0x7DFE,%di
  movw   (%di),%ax
  cmpw   $0xaa55,%ax  # testing the signature
  jne    error
  movw   $0x8100,%di
  movw   (%di),%ax
  cmpw   $0x55aa,%ax  # testing the signature
  jne    error
  movw   $640,%di
  movw   %di,(line_no)   # move to the next line
  movw   $read_okay,%si
  call   print
  ret
 error:
  movw   $read_error,%si
  call   print
  ret

# Main entry where the program runs from
main:
  movw   $welcome_msg1,%si
  call   print
  movw   $welcome_msg2,%si
  call   print

  movw   $crlf,%si
  call   print
  call   read_second_stage
  call   enable_a20
  cli
  call   setup_32
  # call   second_stage
  call   panic


# Text attribute values
.equ BOLD,   0x0F
.equ NORMAL, 0x08
.equ RED,    0x0C
.equ GREEN,  0x0A


.org 0x7CF0
.equ crlf,0x7CF0
  .byte  NORMAL
  .asciz ""

.org 0x7D00
.equ welcome_msg1,0x7D00
  .byte  BOLD
  .asciz " MagicCore RTOS loader"

.org 0x7D20
.equ welcome_msg2,0x7D20
  .byte  NORMAL
  .asciz " Copyright (C) 2023, John Ryland"

.org 0x7D42
.equ read_error,0x7D42
  .byte  RED
  .asciz " Failed"

.org 0x7D50
.equ read_okay,0x7D50
  .byte  GREEN
  .asciz " Passed"

.org 0x7D59
.equ halt_msg,0x7D59
  .byte  RED
  .asciz " Halting"

.org 0x7D63
.equ line_no,0x7D63
  .word 160    # Text position

.org 0x7D68
.equ lba_data,0x7D68
  .byte 0x10, 0x00  # packet size
  .byte 0x03, 0x00  # sectors to read
  .long 0x00007C00  # load address / destination
  .long 0x00000000  # start sector

.org 0x7D78
.equ gdt,0x7D78
   # Null Descriptor
   .word 0x0000, 0x0000
   .word 0x0000, 0x0000
   # .byte 0x00, 0b00000000, 0b00000000, 0x00
   # Code Descriptor
   .word 0xffff, 0x0000
   # .byte 0x00, 0b10011010, 0b11001111, 0x00
   .byte 0x00, 0x9A, 0xCF, 0x00
   # Data Descriptor
   .word 0xffff, 0x0000
   # .byte 0x00, 0b10010010, 0b11001111, 0x00
   .byte 0x00, 0x92, 0xCF, 0x00

.org 0x7D78+28
.equ gdt32ptr,0x7D78+28
   .word 23      # (gdt_end - gdt - 1)
   .long gdt     # (gdt + INITSEG * 0x10 + INITOFF)


# Magic value the BIOS looks for to tell if it is a valid MBR
.org 0x7C00 + 510
  .word 0xAA55

# From below here is the second stage which we load and isn't loaded by the BIOS

.org 0x7E00
second_stage:
  movw   $crlf,%si
  call   print
  movw   $crlf,%si
  call   print
  #call   panic

  movw   $second_start,%si
  call   print

  movw   $crlf,%si
  call   print

  cli
  call   setup_32

  call   panic


#print32:
#  push   $0xb800  # text video memory address
#  pop    %es
#  movw   (line_no),%di
#  lodsb
#  mov    %al,%ah
# print_loop32:
#  lodsb
#  stosw
#  cmp    $0,%al
#  jne    print_loop32
#  movw   (line_no),%di
#  addw   $160,%di
#  movw   %di,(line_no)   # move to the next line
#  # TODO: doesn't handle scrolling the screen if fill the screen
#  ret
#
#second_stage32:
#
#
#
#
#  movw   $second_start,%si
#  call   print32
#
#  movw   $crlf,%si
#  call   print32


.code32

.org 0x7F80
.equ enter_protected_mode,0x7F80


# .code32
#----------------- load CR3 with PML4 -----------------------------------------------
  # Clear the first 0xE000 bytes following 0x1000000.
  movl $0x01000000, %edi
  movl %edi, %cr3
  clrl %eax
  movl $0x0000e000, %ecx
  rep stosl
  movl %cr3, %edi

movl $0x07690748,0xb8000   # Print 'Hi'
hl:
  hlt
  jmp  hl

  # Set the initial page tables.
  # Note that we OR with 0x7 here to allow user-space access, except in the
  # first 2 MiB. We also do this with 0x200 to allow forking the page.

  # Page-Map Level 4
  movl $0x01002000, (%edi)
  addl $0x00001000, %edi

  # Page-Directory Pointer Table
  movl $0x01004000, (%edi)
  addl $0x00001000, %edi

  # Page-Directory (no user-space access here)
  movl $0x01006000, (%edi) # (First 2 MiB)
  movl $0x01008000, 8(%edi) # (Second 2 MiB)
  addl $0x1000, %edi

  # Page-Table
  # Memory map the first 4 MiB.
  clrl %ebx
  movl $1024, %ecx

#SetEntry:
#   mov %ebx, (%edi)
#   add $0x1000, %ebx
#   add $8, %edi
#   loop SetEntry
##----------------- load CR3 with PML4 -----------------------------------------------
#

  xorl   %edi,%edi
  movl   $0x0b8000,%edi
  movb   $'-',%al
  stosb
  stosb
  stosb
  # ret
hl2:
  hlt
  jmp  hl2



.org 0x8000
.equ second_start,0x8000
  .byte  GREEN
  .asciz " Second start"


# Our own magic value for checking the second stage loaded properly
.org 0x8100
  .word 0x55AA

