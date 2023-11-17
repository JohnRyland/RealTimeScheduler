#
# MagicCore x86 Volume Boot Record
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

.text

# CPU starts up in 16-bit mode
.code16

.org 0x7C00

.globl start
start:
  jmp   volume_boot_entry

.org 0x7C04
# Global bootloader parameters
    
default_boot_entry:
boot_entry:
  .byte 0x00

# Delay in milliseconds
#      0 is infinity
#      1 is 1ms
#   1000 is 1s
#  65536 is 65s
bootload_delay_ms:
  .word 800

entry_selector:
  xorl   %eax,%eax
  movb   boot_entry(%eax),%dl

key_pressed:
  mov    $800,%di
  mov    $4,%cx
  xor    %dh,%dh
  movl   $entry_0,%esi

ent:
  pusha

  and    $3,%dl
  xorl   %eax,%eax
  mov    %dl,boot_entry(%eax)
  cmp    %dl,%dh
  
  # draw  '->'  infront of selected item
  jne    ovr
  mov    $0x0F,%ah
  mov    $'-',%al
  stosw
  mov    $'>',%al
  jmp    ovr2
ovr:
  xor    %ax,%ax
  stosw
 ovr2:
  stosw
  add    $2,%di

  mov    $0x0F,%ah
  call   print16
  popa

  inc    %dh
  add    $160,%di
  add    $32,%si
  loop   ent

  xor    %ax,%ax
  int    $0x16
  cmp    $0x48,%ah            # up pressed
  jne    not_up
  dec    %dl
not_up:
  cmp    $0x50,%ah            # down pressed
  jne    not_down
  inc    %dl
not_down:
  cmp    $0x1C,%ah            # enter pressed
  jne    key_pressed
  popa                        # enter / select
  jmp    cont

# welcome_msg:
#   .asciz "MagicCore Volume Boot Record v1.0"

#copyright_msg:
#  .asciz "Copyright (C) 2023, John Ryland"

# This is the logical-block-address descriptor to load 2nd stage / kernel - TODO: merge this in to the parameter block
lba_data:
  # Disk Address Packet (DAP)
  .byte 0x10, 0x00      # Size of DAP, reserved
  .word 0x0040          # Number of sectors to read (seems like a limitation of a max of 127 or 128 depending on the BIOS - or 16-bit segment limit)
lba_data_offset:        # Just to be safe, will limit to loading 64 sectors at a time which is 2kb. This is just for bootloading.
  .word 0x0000          # Linear destination is 0x00007E00 (0x7C00 + 512bytes, BIOS already loads 1st sector for us)
lba_data_segment:       # As a real mode 16bit segment:offset address this could be 0x0000:0x7E00 or  
  .word 0x07E0          # any one of other possibilities, but we choose 0x07E00:0x0000 so INT13h has a full 64k in the offset to load without wrapping.
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
  # Also in ES:DI is PnP info passed to the MBR by the BIOS if it supports
  # this. DL is the drive which is booted still carried through also.

  #  Debug print what is at es:di
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
  movl   $lba_data_sector,%esi   # 32-bit instr to work around clang linker issue
  mov    %ax,(%si)
  mov    %cx,4(%si)

  # Setup Segment Registers continued
  xor    %ax,%ax
  mov    $0xB8,%ah    # 0xb800 text video memory segment address
  mov    %ax,%es
  xorl   %eax,%eax
  mov    bootload_delay_ms(%eax),%cx
delay_loop16:
  pusha
  xor    %cx,%cx
  mov    $0x0100,%dx  # About 1ms
  mov    $0x86,%ah
  int    $0x15        # Wait
  mov    $0x01,%ah
  int    $0x16        # Check if key pressed
  jnz    entry_selector
  popa
  loop   delay_loop16
cont:

  # Setup screen to double line text mode
  # movw   $0x1112,%ax
  # int    $0x10     # Set 80x50 mode (8x8 font)

  # Print Welcome/Version Info
  # movw   $640,%di
  # mov    $0x0A,%ah
  # movl   $welcome_msg,%esi


  # Setup screen, clear
  movw   $0x03,%ax
  int    $0x10     # Set 80x25 mode

  # Print loading kernel ....
  movw   $0,%di
  mov    $0x0F,%ah
  movl   $load_msg,%esi
  call   print16
  sub    $2,%di

  # Read the kernel (or second stage bootloader) directly after the VBR (volume boot record)
read_again:
  mov    $0x0F,%ah
  mov    $'.',%al
  stosw
  push   %edi
  movl   $lba_data,%esi   # Point at the LBA data structure to pass to int 13h
  movb   $0x42,%ah
  int    $0x13            # DL is the drive number, it is passed in at start
  movl   $read_error_msg,%esi
  jc     fatal16
  
  movl   $0x800,%eax      # 0x40 sectors x 512 = 32k, 32k in 16 byte segments = 2k -> 0x800 
  movl   $lba_data_segment,%edi
  addw   %ax,(%edi)       # Set the destination memory address segment where the next disk read should go

  movl   $0x40,%eax       # 0x40 sectors
  movl   $lba_data_sector,%edi
  addl   %eax,(%edi)      # Set the disk sector where the next disk read will start reading from

  pop    %edi
  movl   $kernel_size,%esi
  subw   %ax,(%esi)       # See if we have read enough sectors to have loaded all of the kernel
  cmpw   %ax,(%esi)
  jg     read_again

  # Verify the code loaded as expected
  movl   $sig_mismatch_msg,%esi
  movw   (0x8000 - 2),%ax
  cmpw   $0x55AA,%ax   # testing the signature
  jne    fatal16

  # Verify the code loaded as expected
  #movw   (0x81FE),%ax  # if have a 2nd signature value
  #cmpw   $0x55AA,%ax   # testing the signature
  #jne    fatal16

  # Show Copyright information
  movw   $800,%di
  mov    $0x07,%ah
  # movl   $copyright_msg,%esi
  # call   print16

  movl   $entry_0,%esi
  xor    %edx,%edx
  mov    boot_entry(%edx),%dl
  shl    $5,%dx    # muliply by 32
  add    %dx,%si

  # Jump to the start address of where we loaded the kernel
  jmpw   $0,$0x7E00

read_error_msg:
sig_mismatch_msg:
  .ascii "Error "
# String falls through to 'loading kernel'

load_msg:
  .asciz "loading kernel"

# read_error_msg:
#   .asciz "Disk read error loading kernel!"
#
# sig_mismatch_msg:
#   .asciz "Kernel load error, bad signature!"

#panic_msg:
#  .asciz "Panic!"

print16:
  lodsb
  stosw
  cmp    $0,%al
  jne    print16
  ret

#panic16:
#  movl   $panic_msg,%esi
fatal16:
  # mov    $160,%di
  xorw   %di,%di
  mov    $0x0C,%ah
  call   print16
  # fall-through to halt16

halt16:
  cli
  hlt
  jmp    halt16


.org 0x7C00 + 510 - 128
# Start of parameters block

entry_0:                 # 32 bytes per entry
 kernel_name:            #  Fixed length 8 character name / identifier
   .asciz  "default"     #    the default settings
 kernel_lba_start:       #  LBA sector on disk to start reading from - probably should be defined to be relative to start of the volume
   .long   0x0002        #    sector 2 (MBR in sector 0, VBR in sector 1)
 kernel_size:            #  Size in sectors
   .word   0x0400        #    256*512 = 128k
 kernel_load_address:    #  Where in physical memory to load in segments (x16 for linear address, can be in up to the 1st 1MB of RAM)
   .word   0x07E0        #    loads to 0x00007E00
 kernel_args:            #  Fixed length 16 character list of args
   .asciz  "               "

entry_1:
   .asciz "test   " ; .long 0x0002 ; .word 0x100, 0x00007E00 ; .asciz  "no_vga         "
entry_2:
   .asciz "quiet  " ; .long 0x0002 ; .word 0x100, 0x00007E00 ; .asciz  "quiet no_vga   "
entry_3:
   .asciz "fallbak" ; .long 0x0002 ; .word 0x100, 0x00007E00 ; .asciz  "no_modules     "

# Magic value our MBR looks for to tell if it is a valid VBR
.org 0x7C00 + 510
vbr_boot_signature:
  .word 0xAA55

