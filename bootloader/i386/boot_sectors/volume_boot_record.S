#
# x86 Volume Boot Record
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

# CPU starts up in 16-bit mode
.code16

.org 0x7C00

.globl start
start:
  jmp   volume_boot_entry

welcome_msg:
  .asciz "MagicCore Volume Boot Record v1.0"

copyright_msg:
  .asciz "Copyright (C) 2023, John Ryland"

# This is the logical-block-address descriptor to load 2nd stage
lba_data:
  # Disk Address Packet (DAP)
  .byte 0x10, 0x00      # Size of DAP, reserved
  .word 0x007F          # Number of sectors to read (seems like a limitation of a max of 127 or 128 depending on the BIOS - or 16-bit segment limit)
  .word 0x7E00, 0x0000  # Destination offset and segment (0x7C00 + 512bytes, BIOS already loads 1st sector for us)
lba_data_sector:
  .long 0x0002, 0x0000  # LBA sector on disk to start reading from (64-bit value, we start at 2nd sector as BIOS loaded the 1st one)

  # TODO: if this is a VBR instead of a MBR then we need to copy some
  # of the partition parameters from the partition table entry
  # passed to us in DS:SI in to the data in lba_data.
  #
  # I think something like this as it is just the disk offset needed
  # at the moment:
  #    eax = si[8]
  #    incl eax
  #    lba_data[8] = eax
  #
  # Also in ES:DI is PnP info past to the MBR by the BIOS if it supports
  # this. DL is the drive which is booted still carried through also.

  # mov    %es,%ax
  # mov    %ax,%ds
  # mov    %di,%si
  # mov    $640,%di
  # mov    $0x0A,%ah
  # mov    $0xb800,%ax    # text video memory address
  # mov    %ax,%es
  # lodsb
  # stosw
  # lodsb
  # stosw
  # lodsb
  # stosw
  # lodsb
  # stosw
  # call   print16
  # call   halt16

  # %dl = boot drive unit (we avoided clobbering it)
  # %es:%di = as passed to us by the BIOS. It could point to "$PnP" if BIOS supports this.
  # %ds:%si = points to the partition entry in the MBR we are booting / passing to the VBR

volume_boot_entry:
  # Disable Interrupts
  cli

  # Setup Segment Registers (using cs)
  mov    %cs,%ax      # avoid "push %cs ; pop %ax" because we don't assume the stack is setup
  mov    %ax,%ss

  # Setup Temporary Stack
  mov    $0x7C00,%ax  # 0x7c00 stack location
  mov    %ax,%bp
  mov    %ax,%sp      # a temp stack is setup now

  # Save the params we were passed on entry for later  (saved pushed down from linear address $0x0007C000)
  push   %es
  push   %di
  push   %ds
  push   %si

  # Copy the sector param of the active partition to the lba_data record used to chain load the kernel
  # On entry, %ds:%si points to the partition entry we are booting that the MBR passes to us
  mov    8(%si),%ax
  mov    10(%si),%cx
  inc    %ax
  mov    %cs,%bx
  mov    %bx,%ds
  mov    $lba_data_sector,%esi   # 32-bit instr to work around clang linker issue
  mov    %ax,(%si)
  mov    %cx,4(%si)

  # Setup Segment Registers continued
  xor    %ax,%ax
  mov    $0xB8,%ah    # 0xb800 text video memory segment address
  mov    %ax,%es

  # Print Welcome/Version Info
  movw   $480,%di
  mov    $0x0A,%ah
  movl   $welcome_msg,%esi
  call   print16

  # Read the kernel (or second stage bootloader) directly after the VBR (volume boot record)
  movl   $lba_data,%esi
  movb   $0x42,%ah
  int    $0x13         # DL is the drive number, it is passed in at start
  jc     panic16       # TODO: improve the error messages

  # Verify the code loaded as expected
  movw   (0x7FFE),%ax
  cmpw   $0xAA55,%ax   # testing the signature
  jne    panic16       # TODO: improve the error messages

  # Verify the code loaded as expected
  movw   (0x81FE),%ax
  cmpw   $0x55AA,%ax   # testing the signature
  jne    panic16       # TODO: improve the error messages

  # Enable A20 Gate (needed if plan to use >1MB of RAM)
  movw   $0x2401,%ax
  int    $0x15

  # Show Copyright information
  movw   $640,%di
  mov    $0x07,%ah
  movl   $copyright_msg,%esi
  call   print16

  # Load Global Descriptor Table
  lgdtl   gdt32ptr

  # Setup screen to double line text mode
  movw   $0x1112,%ax
  int    $0x10     # Set 80x50 mode (8x8 font)

  # Mask the IRQs
  #movb   $0x00,%al
	#outb   %al,$0x21
  #movb   $0x00,%al
	#outb   %al,$0xA1

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
  # This is where we continue loading in the kernel's bootstrap code
  ljmpl   $0x0008,$0x7E00
halt32:
  hlt
  jmp    halt32

gdt_msg:
  .asciz "Global Descriptor Table"

# Global descriptor table
gdt:
  # Description of entry:
  #
  #   P - present, D - desc priv lvl / ring, C - 0=sys,1=code/data, T - seg type
  #   G - 4k granularity, B - seg size 0=16bit,1=32bit, L - 64bit code, A - system can use this bit
  #   S - seg limit bits 16-19
  #
  #       seg limit   base 0-15 16-23     PD C T      GBLA  S  base(24-31)
  #       |--------|  |--------|-----|    |/\|/--\    |||||--|  |--|
  #       0xff, 0xff, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Example Descriptor
  #
  .byte   0x00, 0x00, 0x00, 0x00, 0x00, 0b00000000, 0b00000000, 0x00  # Null Descriptor
  .byte   0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Code Descriptor
  .byte   0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10010010, 0b11001111, 0x00  # Data Descriptor
  .byte   0x68, 0x00, 0x00, 0x00, 0x00, 0b10001001, 0b11001111, 0x00  # TSS Descriptor
  #
  # Same but only in hexidecimal for reference:
  #
  #  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Null Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00  # Code Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00  # Data Descriptor
  #  .byte 0x68, 0x00, 0x00, 0x00, 0x00, 0x89, 0xCF, 0x00  # TSS Descriptor

# Global descriptor table size and pointer
.org 0x7C00 + 510 - 6
.equ gdt32ptr,0x7C00 + 510 - 6
  .word 31    # Size-1
  .long gdt   # Location

# Magic value our MBR looks for to tell if it is a valid VBR
.org 0x7C00 + 510
vbr_boot_signature:
  .word 0xAA55
