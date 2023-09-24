#
# x86 OS Bootloader
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

# CPU starts up in 16-bit mode
.code16

# The code is loaded by the BIOS at 0x7C00 so make our addresses match up
# An alternative approach is to "ljmp $0x07C0,offset" where this sets cs.
.org 0x7C00

# Start of the Bootloader
bootloader_entry:
  jmp    start

# Global descriptor table size and pointer
# .org 0x7C02
.equ gdt32ptr,0x7C02
  .word 23    # Size-1
  .long gdt   # Location

# Global descriptor table
gdt:
  # .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Null Descriptor
  # .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00  # Code Descriptor
  # .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00  # Data Descriptor
  # .byte 0x68, 0x00, 0x00, 0x00, 0x00, 0x89, 0xCF, 0x00  # TSS Descriptor

  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0b00000000, 0b00000000, 0x00  # Null Descriptor
  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Code Descriptor
  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10010010, 0b11001111, 0x00  # Data Descriptor
  .byte 0x68, 0x00, 0x00, 0x00, 0x00, 0b10001001, 0b11001111, 0x00  # TSS Descriptor

  # Description of entry:
  #       seg limit   base 0-15 16-23     PD S T      GBLA  S  base(24-31)
  #       |--------|  |--------|-----|    |/\|/--\    |||||--|  |--|
  # .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Example Descriptor
  #
  #   P - present, D - desc priv lvl / ring, S - 0=sys,1=code/data, T - seg type
  #   G - 4k granularity, B - seg size 0=16bit,1=32bit, L - 64bit code, A - system can use this bit
  #   S - seg limit bits 16-19

# This is the logical-block-address descriptor to load 2nd stage
lba_data:
  # Disk Address Packet (DAP)
  .byte 0x10, 0x00      # Size of DAP, reserved
  .word 0x007F          # Number of sectors to read (seems like a limitation of a max of 127 or 128 depending on the BIOS)
  .word 0x7E00, 0x0000  # Destination offset and segment (0x7C00 + 512bytes, BIOS already loads 1st sector for us)
  .long 0x0001, 0x0000  # LBA sector on disk to start reading from (64bit value, we start at 2nd sector as BIOS loaded the 1st one)

welcome_msg1:
  .asciz "MagicCore RTOS v1.0"

welcome_msg2:
  .asciz "Copyright (C) 2023, John Ryland"

.globl start
start:
  # Disable Interrupts
  cli

  # Setup Segment Registers (using cs)
  movw   %cs,%ax      # avoid "push %cs ; pop %ax" because we don't assume the stack is setup
  movw   %ax,%ds
  movw   %ax,%ss
  movw   $0xb800,%ax  # text video memory address
  movw   %ax,%es

  # Setup Temporary Stack
  mov    $0x7C00,%bp
  mov    $0x7C00,%sp

  # Setup and clear the screen
  movw   $0x03,%ax
  int    $0x10         # set screen to VGA text mode 03h

  # Print Welcome/Version Info
  movw   $324,%di
  mov    $0x0A,%ah
  movl   $welcome_msg1,%esi
  call   print16

  # Read the kernel (or second stage bootloader) directly after the MBR (master boot record)
  movl   $lba_data,%esi
  movb   $0x42,%ah
  int    $0x13         # DL is the drive number, it is passed in at start
  jc     panic16

  # Verify the code loaded as expected
  movw   (0x7DFE),%ax
  cmpw   $0xAA55,%ax  # testing the signature
  jne    panic16

  # Verify the code loaded as expected
  movw   (0x81FE),%ax
  cmpw   $0x55AA,%ax  # testing the signature
  jne    panic16

  # Enable A20 Gate (needed if plan to use >1MB of RAM)
  movw   $0x2401,%ax
  int    $0x15

  # Show Copyright information
  movw   $644,%di
  mov    $0x0F,%ah
  movl   $welcome_msg2,%esi
  call   print16

  # Load Global Descriptor Table
  lgdtl   gdt32ptr

  # Setup screen to double line text mode
  movw   $0x1112,%ax
  int    $0x10     # Set 80x50 mode (8x8 font)

  # Enable 32-bit Protected Mode (set bit 0 in CR0)
  movl   %cr0,%eax
  orl    $(1<<0),%eax
  movl   %eax,%cr0

  # Setup Segment Registers
  xorl   %eax,%eax
  movl   %eax,%fs
  movl   %eax,%gs
  movb   $0x10,%al
  movl   %eax,%ds
  movl   %eax,%es
  movl   %eax,%ss

  # Reload Code Segment and Instruction Pointer (switches to 32-bit mode)
  ljmpl   $0x0008,$enter_protected_mode

print16:
  lodsb
  stosw
  cmp    $0,%al
  jne    print16
  ret

halt_msg:
  .asciz "Panic!"

panic16:
  xorw   %di,%di
  movl   $halt_msg,%esi
  mov    $0x0C,%ah
  call   print16
  # fall-through to halt16

halt16:
  cli
  hlt
  jmp    halt16

# Code switches to 32-bit code here
.code32
enter_protected_mode:
  call   _start32

  # load CR3 with PML4
  # Clear the first 0xE000 bytes following 0x1000000.
  movl   $0x01000000,%edi
  movl   %edi,%cr3
  xorl   %eax,%eax
  movl   $0x0000e000,%ecx
  rep    stosl
  movl   %cr3,%edi

  # Set the initial page tables.
  # Note that we OR with 0x7 here to allow user-space access, except in the
  # first 2 MiB. We also do this with 0x200 to allow forking the page.

  xorl   %eax,%eax
  incl   %eax
  shll   $12,%eax                 #  eax = $0x00001000 (4K)
  movl   $0x01002000,%ebx

  # Page-Map Level 4
  # movl   $0x01002000,(%edi)     #   [%edi] = $0x01002000
  # addl   $0x00001000,%edi
  movl   %ebx,(%edi)
  addl   %eax,%ebx
  addl   %eax,%ebx
  addl   %eax,%edi

  # Page-Directory Pointer Table
  # movl   $0x01004000,(%edi)     #   [%edi + 4096] = $0x01004000
  # addl   $0x00001000,%edi
  movl   %ebx,(%edi)
  addl   %eax,%ebx
  addl   %eax,%ebx
  addl   %eax,%edi

  # Page-Directory (no user-space access here)
  movl   $0x01006000,(%edi)   # (First 2 MiB)    #   [%edi + 8192] = $0x01006000
  movl   $0x01008000,8(%edi)  # (Second 2 MiB)   #   [%edi + 8192 + 8] = $0x01008000
  addl   %eax,%edi
  # Page-Table, Memory map the first 4 MiB.
  clrl   %ebx

  movl   $1024,%ecx  # 1024 entries each 4kb which is 4MB
 set_entry:
  movl   %ebx,(%edi)
  addl   %eax,%ebx
  add    $8,%edi
  loop   set_entry

  movl   $0x01008000,%eax  # (Second 2 MiB)
  movl   %eax,%esp

  call   _load_idt

  # Remap the IRQs - this could be moved to C code
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

  call   _start32

# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
.org 0x7C00 + 510
  .word 0xAA55

.globl load_idt
load_idt:
.globl _load_idt
_load_idt:
  # Load Interrupt Descriptor Table
  lidt   idt32ptr
  ret

# Interrupt descriptor table
idt:
  .word isr_entry_1+ 0*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 1*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 2*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 3*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 4*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 5*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 6*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 7*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 8*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+ 9*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+10*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+11*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+12*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+13*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+14*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+15*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+16*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+17*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+18*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+19*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+20*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+21*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+22*5+0, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+23*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+24*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+25*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+26*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+27*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+28*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+29*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+30*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+31*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+32*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+33*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+34*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+35*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+36*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+37*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+38*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+39*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+40*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+41*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+42*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+43*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+44*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+45*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+46*5+2, 0x0008, 0b1110111011101110, 0x0000
  .word isr_entry_1+47*5+2, 0x0008, 0b1110111011101110, 0x0000
idt_end:

idt32ptr:
  .word idt_end - idt - 1
  .long idt

.org 0x8102
.equ isr_entry_1,0x8102
# isr_entry_1:
  .byte 0x60,0x6a,0x00,0xeb,22*5  # pushal ; pushl $INTERRUPT_NUM ; jmp double_jump
  .byte 0x60,0x6a,0x01,0xeb,21*5
  .byte 0x60,0x6a,0x02,0xeb,20*5
  .byte 0x60,0x6a,0x03,0xeb,19*5
  .byte 0x60,0x6a,0x04,0xeb,18*5
  .byte 0x60,0x6a,0x05,0xeb,17*5
  .byte 0x60,0x6a,0x06,0xeb,16*5
  .byte 0x60,0x6a,0x07,0xeb,15*5
  .byte 0x60,0x6a,0x08,0xeb,14*5
  .byte 0x60,0x6a,0x09,0xeb,13*5
  .byte 0x60,0x6a,0x0a,0xeb,12*5
  .byte 0x60,0x6a,0x0b,0xeb,11*5
  .byte 0x60,0x6a,0x0c,0xeb,10*5
  .byte 0x60,0x6a,0x0d,0xeb, 9*5
  .byte 0x60,0x6a,0x0e,0xeb, 8*5
  .byte 0x60,0x6a,0x0f,0xeb, 7*5
  .byte 0x60,0x6a,0x10,0xeb, 6*5
  .byte 0x60,0x6a,0x11,0xeb, 5*5
  .byte 0x60,0x6a,0x12,0xeb, 4*5
  .byte 0x60,0x6a,0x13,0xeb, 3*5
  .byte 0x60,0x6a,0x14,0xeb, 2*5
  .byte 0x60,0x6a,0x15,0xeb, 1*5
  .byte 0x60,0x6a,0x16,0xeb, 0*5
double_jump:
  jmp    generic_isr
  .byte 0x60,0x6a,0x17,0xeb,24*5  # pushal ; pushl $INTERRUPT_NUM ; jmp generic_isr
  .byte 0x60,0x6a,0x18,0xeb,23*5
  .byte 0x60,0x6a,0x19,0xeb,22*5
  .byte 0x60,0x6a,0x1a,0xeb,21*5
  .byte 0x60,0x6a,0x1b,0xeb,20*5
  .byte 0x60,0x6a,0x1c,0xeb,19*5
  .byte 0x60,0x6a,0x1d,0xeb,18*5
  .byte 0x60,0x6a,0x1e,0xeb,17*5
  .byte 0x60,0x6a,0x1f,0xeb,16*5
  .byte 0x60,0x6a,0x20,0xeb,15*5
  .byte 0x60,0x6a,0x21,0xeb,14*5
  .byte 0x60,0x6a,0x22,0xeb,13*5
  .byte 0x60,0x6a,0x23,0xeb,12*5
  .byte 0x60,0x6a,0x24,0xeb,11*5
  .byte 0x60,0x6a,0x25,0xeb,10*5
  .byte 0x60,0x6a,0x26,0xeb, 9*5
  .byte 0x60,0x6a,0x27,0xeb, 8*5
  .byte 0x60,0x6a,0x28,0xeb, 7*5
  .byte 0x60,0x6a,0x29,0xeb, 6*5
  .byte 0x60,0x6a,0x2a,0xeb, 5*5
  .byte 0x60,0x6a,0x2b,0xeb, 4*5
  .byte 0x60,0x6a,0x2c,0xeb, 3*5
  .byte 0x60,0x6a,0x2d,0xeb, 2*5
  .byte 0x60,0x6a,0x2e,0xeb, 1*5
  .byte 0x60,0x6a,0x2f,0xeb, 0*5
generic_isr:
  cli    # disable interrupts
  cld
  call   _interrupt_handler  # Up to the handler check the In-Service-Register and send EOI as required
  popl   %eax
  popal
  iret

# Our own magic value for checking the second stage loaded properly
.org 0x81FE
  .word 0x55AA
