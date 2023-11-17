#
# MagicCore x86 Master Boot Record
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

# CPU starts up in 16-bit mode
.code16

# The code is loaded by the BIOS at 0x7C00 so make our addresses match up
# An alternative approach is to "ljmp $0x07C0,offset" where this sets cs.
.org 0x7C00

# Start of the Bootloader
.globl start
start:
  jmp    bootloader_entry

welcome_msg:
  # .asciz "MagicCore MBR v1.0"
  .asciz "MagicCore Master Boot Record v1.0"

copyright_msg:
  .asciz "Copyright (C) 2023, John Ryland"

# This is working space for the logical-block-address descriptor to load a partition's VBR
lba_data:
  # Disk Address Packet (DAP)
  .byte 0x10, 0x00      # Size of DAP, reserved
  .word 0x0001          # Number of sectors to read
  .word 0x7C00, 0x0000  # Destination offset and segment (0x0000:0x7C00 is where a MBR or VBR is expected to be run from)
lba_data_sector:
  .long 0x0000, 0x0000  # LBA sector on disk to start reading from (64-bit value, will be set based on the partition table)

error_loading_partition_msg:
  .ascii "Error "
loading_partition_msg:
  .ascii "loading "
loading_partition_number:
  .ascii "Nth "
loading_partition_msg_end:
  .asciz "partition"

partition_numbers:
  .ascii "4th "
  .ascii "3rd "
  .ascii "2nd "
  .ascii "1st "

no_active_partition_msg:
  .asciz "No active partition!"
  # .asciz "No active partition found in the partition table!"

invalid_volume_boot_record:
  .asciz "Active partition has invalid VBR!"
  # .asciz "Active partition has an invalid VBR (Volume Boot Record)!"

bootloader_entry:
  # Disable Interrupts
  cli

  # Setup Segment Registers
  xor    %ax,%ax        # avoid stack until the stack is setup
  mov    %ax,%ds
  mov    %ax,%ss

  # Setup Temporary Stack - perhaps should consider if 0xFFFF is better
  mov    $0x7C,%ah
  mov    %ax,%bp
  mov    %ax,%sp        # our stack is now ready to use (offset has room to grow down)
  push   %di
  push   %es            # save %es and %di to pass on to the VBR (PnP BIOS extensions)

  # Relocate this MBR to 4096 bytes higher so we can load VBR in to this location
  mov    $0x1000,%cx    # 4096, large enough if our sectors are 4k instead of 512
  mov    %ax,%si        # copy with src from bootloader_entry
  mov    %ax,%di
  add    %cx,%di        # copy dst is 4096 bytes higher in memory
  xor    %ax,%ax
  mov    %ax,%es        # we can now clobber %es
  rep    movsb          # do the asm version of memcpy

  # BIOSes are meant to set %cs:%ip to $0x0000:$0x7C00 but some BIOSes incorrectly use $0x07C0:$0x0000 which
  # points to the same memory, but we don't make any assumptions about what those registers are set to, and then
  # below here we explicitly set these with the long/far jump removing any ambiguity.
  # Unfortunately we use a 32-bit instr to work around clang linker issue, but perhaps we can hexedit out 0x66
  # and replace with 0x90, to convert the instruction prefix with a nop.

  # Jump to where we relocated the MBR - a segment of 0x100 is an offset of 0x1000 (4096)
  jmpl   $0x100,$continue_loading  # 32-bit instr to work around clang linker issue

# We will continue execution here at the address we have relocated to
continue_loading:
  # Re-setup data segment registers for the new location
  mov    $0x01,%ah      # a segment of 0x0100 is an offset of 0x1000 (4096)
  mov    %ax,%ds        # our reference to variables will now be correct when using the %ds segment
  mov    $0xb8,%ah      # 0xb800 text video memory address
  mov    %ax,%es

  # Setup and clear the screen
  movw   $0x0003,%ax
  int    $0x10          # set screen to VGA text mode 03h

  # Print welcome/version info
  xor    %di,%di
  mov    $0x0A,%ah
  mov    $welcome_msg,%esi  # 32-bit instr to work around clang linker issue
  call   print16

  # Show Copyright information
  mov    $160,%di
  mov    $0x07,%ah
  mov    $copyright_msg,%esi  # 32-bit instr to work around clang linker issue
  call   print16

  # Scan the partition table for the active partition
  mov    $0x04,%cx
  mov    $partition_1_flags,%esi  # 32-bit instr to work around clang linker issue
next_partition:
  lodsb
  cmpb   $0x80,%al
  je     found_partition
  add    $0x000F,%si
  loop   next_partition

no_active_partition_found:
  movl   $no_active_partition_msg,%esi   # 32-bit instr to work around clang linker issue
  jmp    critical_error

found_partition:

  dec    %si
  push   %si   # Save

  # Copy the sector param of the active partition to the lba_data record used to load the VBR
  push   %di
  push   %es
  push   %ds
  pop    %es
  mov    $lba_data_sector,%edi   # 32-bit instr to work around clang linker issue
  add    $0x0008,%si  # offset from where we found the active flag to the position in the record of the LBA offset
  movsw
  movsw
  # Copy the string corresponding to the index of the partition to the output string
  movl   $loading_partition_number,%edi   # 32-bit instr to work around clang linker issue
  xor    %eax,%eax
  leal   partition_numbers-4(%eax,%ecx,4),%esi   # 32-bit instr to work around clang linker issue
  movsw
  movsw
  pop    %es
  pop    %di

  # Print the info about which partition we are about to boot
  mov    $loading_partition_msg,%esi      # 32-bit instr to work around clang linker issue
  mov    $0x08,%ah
  mov    $480,%di
  call   print16

  # Check we have int13h BIOS extensions
  mov    $0x41,%ah
  mov    $0x55AA,%bx
  int    $0x13          # DL is the drive number, it is passed in at start
  movl   $error_loading_partition_msg,%esi   # 32-bit instr to work around clang linker issue
  cmp    $0xAA55,%bx
  jne    critical_error

  mov    $(0x0800+'.'),%ax
  stosw
  push   %ax

#  # Check the sector size (load extended drive parameter table)
#  mov    $0x48,%ah
#  push   %ds
#  mov    $0x07E0,%ax
#  mov    %ax,%ds
#  xor    %si,%si        # DS:SI -> buffer for drive parameters, word at offset 0x18 has the sector size
#  int    $0x13          # DL is the drive number, it is passed in at start
#  pop    %ds
#  jnc    not_carry      # CF set on error
#  movl   $error_loading_partition_msg,%esi   # 32-bit instr to work around clang linker issue
#  jmp    critical_error
#not_carry:

  # Read the VBR (volume boot record) in to memory directly after where this MBR (master boot record) is.
  mov    $lba_data,%esi # 32-bit instr to work around clang linker issue
  mov    $0x42,%ah
  int    $0x13          # DL is the drive number, it is passed in at start
  jnc    load_success 

  movl   $error_loading_partition_msg,%esi   # 32-bit instr to work around clang linker issue
  jmp    critical_error
load_success:

  pop    %ax
  stosw
#  push   %ax

  # Verify the code loaded as expected
  movl   $invalid_volume_boot_record,%esi   # 32-bit instr to work around clang linker issue
  push   %ds
  xor    %ax,%ax
  mov    %ax,%ds
  mov    (0x7DFE),%ax
  pop    %ds
  cmp    $0xAA55,%ax    # testing the signature
  jne    critical_error

#  pop    %ax
#  stosw

  # Restore registers so we can pass the following through to the VBR:
  # %dl = boot drive unit (we avoided clobbering it)
  # %es:%di = as passed to us by the BIOS. It could point to "$PnP" if BIOS supports this.
  # %ds:%si = points to the partition entry in the MBR we are booting / passing to the VBR
  pop    %si
  pop    %es
  pop    %di

  # Jump to where the VBR was loaded
  jmpl   $0x00,$0x7C00  # 32-bit instr to work around clang linker issue


# Our mini runtime environment for our boot-loader

print_loop:
  stosw
print16:                # prints str in %si to screen offset in %di with attrib from %ah
  lodsb
  cmp    $0,%al
  jne    print_loop
  ret

critical_error:         # prints message in %si in red then halts
  mov    $0x0C,%ah
  call   print16
  # fall-through to halt16

halt16:                 # stop executing, freeze
  cli
  hlt
  jmp    halt16


partiton_table_msg:
#  .asciz "Partition Table"

p_start:
  .asciz ".parttab"

# If this is to be a MBR then we need a partition table
.org 0x7C00 + 510 - 4*16
.globl _partition_table
_partition_table:

partition_1_flags:
  .byte 0x80   # Active
partition_1_start_CHS_address:   # Cylinder, Head, Sector
  .byte 0, 0, 0       # H, S/C, C
partition_1_type:
  .byte 0
partition_1_end_CHS_address:
  .byte 0, 0, 0
partition_1_start_LBA:
  .long 0x01   # Starts after the MBR
partition_1_number_of_sectors:
  .long 0x80   # 64 kb

partition_2_flags:
  .byte 0x00   # Inactive
partition_2_start_CHS_address:
  .byte 0, 0, 0
partition_2_type:
  .byte 0
partition_2_end_CHS_address:
  .byte 0, 0, 0
partition_2_start_LBA:
  .long 0x00   # Starts after the partition 1
partition_2_number_of_sectors:
  .long 0x00

partition_3_flags:
  .byte 0x00   # Inactive
partition_3_start_CHS_address:
  .byte 0, 0, 0
partition_3_type:
  .byte 0
partition_3_end_CHS_address:
  .byte 0, 0, 0
partition_3_start_LBA:
  .long 0
partition_3_number_of_sectors:
  .long 0

partition_4_flags:
  .byte 0x00    # Inactive
partition_4_start_CHS_address:
  .byte 0, 0, 0
partition_4_type:
  .byte 0
partition_4_end_CHS_address:
  .byte 0, 0, 0
partition_4_start_LBA:
  .long 0
partition_4_number_of_sectors:
  .long 0


# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
# When testing 4k sector sizes with qemu, it appears the BIOS checks the fixed offset rather than
# relative to the end of the physical sector. Logical sector size seems to always be 512 bytes.
.org 0x7C00 + 510
boot_signature:
  .word 0xAA55


# TODO: Some disks have 4096 byte sectors, we currently have no
# detection of the sector size.
#
# TODO: Add support for GPTs (GUID Partition Tables) if wanting
# to support disks greater than 2TB.
#
# NOTE: 2TB is assuming 512 byte sectors and 32-bit LBA sector
# offsets and sizes. 4096 byte sector sizes extends this to up
# to 16TB, but without detection, we assume sectors are 512 bytes.
