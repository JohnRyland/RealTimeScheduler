#
# x86 OS Bootloader
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

# CPU starts up in 16-bit mode
.code16

# The code is loaded by the BIOS at 0x7C00 so make our addresses match up
.org 0x7C00

# Setup the registers and then jump to main
.globl start
start:
#  cli
#  movw  %cs,%ax   # Setup segment registers
#  movw  %ax,%ds
#  movw  %ax,%es
#  movw  %ax,%ss
#  mov bp,7c00h
#  mov sp,7c00h    # Setup a stack
#  sti
  push   %cs
  pop    %ds
  movw   $0x03,%ax
  int    $0x10     # reset the screen mode
  movw   $0x1112,%ax
  int    $0x10     # set 80x50 mode
  jmp    main16

# Text attribute values
.equ BOLD,   0x0F
.equ NORMAL, 0x08
.equ RED,    0x0C
.equ GREEN,  0x0A

.org 0x7C0F
.equ crlf,0x7C0F
  .byte  NORMAL
  .asciz ""

.org 0x7C11
.equ welcome_msg1,0x7C11
  .byte  BOLD
  .asciz "MagicCore RTOS v1.0"

.org 0x7C26
.equ welcome_msg2,0x7C26
  .byte  NORMAL
  .asciz "Copyright (C) 2023, John Ryland"

.org 0x7C47
.equ read_error,0x7C47
  .byte  RED
  .asciz "ReadErr"

.org 0x7C51
.equ read_okay,0x7C51
  .byte  GREEN
  .asciz "[X] Read"

.org 0x7C5A+2
.equ halt_msg,0x7C5A+2
  .byte  RED
  .asciz "Panic!"

.org 0x7C65+2
.equ line_no,0x7C65+2
  .long 160    # Text position

# This is the logical-block-address descriptor to load 2nd stage
.org 0x7C69+2
.equ lba_data,0x7C69+2
  .byte 0x10, 0x00  # packet size
  .byte 0x40, 0x00  # sectors to read
  .long 0x00007C00  # load address / destination
  .long 0x00000000  # start sector

.org 0x7C75+2
.equ gdt,0x7C75+2
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0b00000000, 0b00000000, 0x00  # Null Descriptor
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Code Descriptor
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0b10010010, 0b11001111, 0x00  # Data Descriptor

.org 0x7C75+30
.equ gdt32ptr,0x7C75+30
  .word 23
  .long gdt

.org 0x7C99
.equ a20_okay_msg,0x7C99
  .byte GREEN
  .asciz "[X] A20"

.org 0x7C99+10
.equ gdt_okay_msg,0x7C99+10
  .byte GREEN
  .asciz "[X] GDT"

.org 0x7C99+20
.equ pm_okay_msg, 0x7C99+20
  .byte GREEN
  .asciz "[X] 32-bit"

.org 0x7CB9
.equ pagemap_okay_msg, 0x7CB9
  .byte GREEN
  .asciz "[X] PageMap"

.org 0x7CB9+14
.equ cr3_okay_msg, 0x7CB9+14
  .byte GREEN
  .asciz "[X] CR3"

# Prints null terminated string %si direct to VGA memory
# First byte pointed to is the text attribute byte to use
print16:
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
  addw   $160,(line_no)  # move to the next line
  ret

panic16:
  movw   $0,(line_no)    # show at top of screen
  call   print16
 halt16:
  hlt
  jmp    halt16

# Main entry where the program runs from
main16:
  # Print welcome/version info
  movw   $welcome_msg1,%si
  call   print16
  movw   $welcome_msg2,%si
  call   print16

  # Read the second stage boot loader after the master boot record
  movw   $lba_data,%si
  movb   $0x42,%ah
  int    $0x13

  movw   $read_error,%si
  jc     panic16

  movw   (0x7DFE),%ax
  cmpw   $0xAA55,%ax  # testing the signature
  jne    panic16

  movw   (0x8100),%ax
  cmpw   $0x55AA,%ax  # testing the signature
  jne    panic16

  movw   $640,(line_no)
  movw   $read_okay,%si
  call   print16

  # Clear interrupt flag
  cli

  # Notify BIOS of our intended mode (32/64 bit mode)
  movw   $0xEC00,%ax
  movb   $1,%bl      #  1 for 32-bit, 2 for 64-bit, 3 for mixed
  int    $0x15

  # Enable A20 Gate
  movw   $0x2401,%ax
  int    $0x15
  
  movw   $a20_okay_msg,%si
  call   print16

  # Load Global Descriptor Table
  lgdt   gdt32ptr

  movw   $gdt_okay_msg,%si
  call   print16

  # Setup 32bit mode
  # Load Control Register CR0
  # Enable Protected Mode (Bit0 in CR0)
  # Store Control Register CR0
  movl   %cr0,%eax
  orl    $(1<<0),%eax
  movl   %eax,%cr0

  # Setup all Segment Registers
  # and reload Code Segment, Instruction Pointer
  xorl   %eax,%eax
  movl   %eax,%fs
  movl   %eax,%gs
  movb   $0x10,%al
  movl   %eax,%ds
  movl   %eax,%es
  movl   %eax,%ss
  ljmp   $0x0008,$(enter_protected_mode)  # Sets %cs to 0x08 and switches to 32-bit mode

# Code switches to 32-bit code here
.code32
.org 0x7D68
.equ enter_protected_mode,0x7D68

  movl   $pm_okay_msg,%esi
  call   print32

  # Jump to the code in the 2nd disk sector
  ljmp   $0x0008,$(enter_second_stage)

  # Panic and stop running if we fall through
 panic32:
  xorl   %eax,%eax
  movl   %eax,(line_no)
  movl   $halt_msg,%esi
  call   print32
 halt32:
  hlt
  jmp    halt32

# Prints null terminated string %si direct to VGA memory
# First byte pointed to is the text attribute byte to use
print32:
  movl   $0xb8000,%edi    # text video memory address
  addl   (line_no),%edi
  lodsb
  mov    %al,%ah
 print_loop32:
  lodsb
  stosw
  or     %al,%al
  jne    print_loop32
  movl   (line_no),%edi
  addl   $160,%edi
  movl   %edi,(line_no)   # move to the next line
  # Note: doesn't handle scrolling the screen so don't print more than 24 lines
  ret


# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
.org 0x7C00 + 510
  .word 0xAA55


.org 0x7E00
# From below here is the second stage which we load and wasn't loaded by the BIOS

.org 0x7E10
.equ enter_second_stage,0x7E10

  movl   $second_stage_okay_msg,%esi
  call   print32

  # load CR3 with PML4
  # Clear the first 0xE000 bytes following 0x1000000.
  movl   $0x01000000,%edi
  movl   %edi,%cr3
  clrl   %eax
  movl   $0x0000e000,%ecx
  rep    stosl
  movl   %cr3,%edi
  
  movl   $cr3_okay_msg,%esi
  call   print32

  # Set the initial page tables.
  # Note that we OR with 0x7 here to allow user-space access, except in the
  # first 2 MiB. We also do this with 0x200 to allow forking the page.

  # Page-Map Level 4
  movl   $0x01002000,(%edi)
  addl   $0x00001000,%edi
  # Page-Directory Pointer Table
  movl   $0x01004000,(%edi)
  addl   $0x00001000,%edi
  # Page-Directory (no user-space access here)
  movl   $0x01006000,(%edi)   # (First 2 MiB)
  movl   $0x01008000,8(%edi)  # (Second 2 MiB)
  addl   $0x1000,%edi
  # Page-Table, Memory map the first 4 MiB.
  clrl   %ebx

  movl   $1024,%ecx  # 1024 entries each 4kb which is 4MB
 set_entry:
  mov    %ebx,(%edi)
  add    $0x1000,%ebx
  add    $8,%edi
  loop   set_entry

  movl   $pagemap_okay_msg,%esi
  call   print32

  movl   $0x01008000,%eax  # (Second 2 MiB)
  movl   %eax,%esp

  # Load Interrupt Descriptor Table
  lidt   idt32ptr
  
  # Remap the IRQs
  movb   $0x11,%al
	outb   %al,$0x20  # starts the initialization sequence (in cascade mode)
	outb   %al,$0xA0
  movb   $0x20,%al
	outb   %al,$0x21
  movb   $0x28,%al
	outb   %al,$0xA1  # 0x01C0
  movb   $0x04,%al
	outb   %al,$0x21  # ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  movb   $0x02,%al
	outb   %al,$0xA1  # ICW3: tell Slave PIC its cascade identity (0000 0010)
  movb   $0x01,%al
	outb   %al,$0x21  # ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb   %al,$0xA1

  # Unmask the IRQs
  movb   $0xfc,%al  # unmask the timer and keyboard interrupts
	outb   %al,$0x21  # ICW4: have the PICs use 8086 mode (and not 8080 mode)
  movb   $0xff,%al
	outb   %al,$0xA1

  jmp    _start

.globl _start
_start:
  # Entry in to our C code (shouldn't return control)
  call   _start32
  jmp    panic32

.org 0x8000
.equ second_stage_okay_msg,0x8000
  .byte  GREEN
  .asciz "[X] 2nd Stage"

#
# Space here for any asm code needed for early start
#

.org 0x8020
.equ cpu_fault,0x8020
  pushal
  cld
  movl   $0xb8000,%edi    # text video memory address
  mov    $0x0A,%ah
  mov    $'X',%al
  stosw
  popal
  iret

# .globl interrupt_service_routine
.org 0x8040
.equ timer_func,0x8040
# _isr_func:
# interrupt_service_routine:
  pushal
  cld

  call   get_isr
  cmpb   $0,%al
  je     fini      # See if it was spurious

  pushl  %eax      # pass the ISR as first param
  call   _interrupt_handler
  popl   %eax

 eoi:
  movb   $0x20,%al
  outb   %al,$0x20  # Send EOI (End-of-interrupt)
 fini:
  popal
  iret

get_isr:
  xor    %eax,%eax
  movb   $0x0b,%al
  outb   %al,$0x20
  inb    $0x20     # Get ISR (In-Service-Register)
  ret

.org 0x8062
.equ keyboard_func,0x8062
  pushal
  cld
  
  movl   $0xb8000,%edi    # text video memory address
  movw   (%edi),%ax
  mov    $0x0A,%ah
  cmp    $'K',%al
  jne    over1
  mov    $'X',%al
  jmp    over2
 over1:
  mov    $'K',%al
 over2:
  stosw

  call   get_isr
  cmpb   $0,%al
  je     fini      # See if it was spurious
  jmp    eoi


# Our own magic value for checking the second stage loaded properly
.org 0x8100
  .word 0x55AA


.org 0x8110
.equ idt32ptr,0x8110
  .word 8*34-1
  .long idt


.org 0x8120
.equ idt,0x8120
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000
  .word cpu_fault, 0x0008, 0b1110111011101110, 0x0000  
  .word timer_func, 0x0008, 0b1110111000000000, 0x0000
  .word keyboard_func, 0x0008, 0b1110111000000000, 0x0000
  # .word 0x0000, 0b1110111000000000, 0x0008, isr_func


