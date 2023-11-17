#
# MagicCore x86 OS Kernel Loader
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

#include "macros.S"

# First it needs to be loaded in real mode (16-bit) addressable memory.
# Without enabling A20 gate, it would need to be in the first 1MB of memory
# but loaded in up to 64kb chunks at a time with a 1MB size limit.
# Enabling A20 would mean larger kernels if needed (seems large).
# Then this memory can be mapped by the page tables / GDT to some specific
# address. Page tables belong in the kernel, so need to decide what the
# load address means here. The toolchain linker can relocate the addresses
# for running at a specific address, but we might need two sections of code,
# one at the pre-page table set up location, and one after. Need to think
# about this.
# For now, easiest is just run it from where we load it to, but enable A20
# so it can be higher in memory. (we might also need to detect how much physical
# memory we actually have too before we start mapping memory. I think it is
# fair to assume atleast 1MB though, this isn't really designed for 286 or
# earlier, really minimum is 386 but realistically much higher).

.text
.org 0x7E00
.code16
GLOBAL(enter_16):
  # call instead of jmp helps with setting up a call-stack
  pushw  $0x0         # fake it being a 32-bit return address
  call   start_16

# Global descriptor table size and pointer
.org 0x7E05
.equ gdt32ptr,0x7E05
  .word  39      # Size-1
  .long  gdt_32  # Location

welcome_msg:
  .asciz "MagicCore RTOS v1.0"

copyright_msg:
  .asciz "Copyright (C) 2023, John Ryland"

GLOBAL(start_16):
  call   enable_32

GLOBAL(enter_32):
  # Reload Code Segment and Instruction Pointer (switches to 32-bit mode)
  calll  $0x0008,$protected_mode_entry_32

# Code switches to 32-bit code here
.code32
protected_mode_entry_32:

  # This passes in to start32 a pointer to the parameter block we got from the VBR
  # pushl  %esi
  movl   %esi,bootloader_parameter_block_ptr

  movl   $welcome_msg,%esi
  movb   $0x0A,%ah
  call   print_32

  movl   $copyright_msg,%esi
  movb   $0x07,%ah
  call   print_32

  # call all the early initializer pointers that have been added
  movl   $init_early_start,%esi
call_initializers_loop:
  cmpl   $init_early_end,%esi
  je     done_inits
  movl   (%esi),%eax
  pushal
  calll  *%eax        # Assumes %esi doesn't get clobbered
  popal
  addl   $0x4,%esi
  jmp    call_initializers_loop
done_inits:

GLOBAL(start_32):
  # This is where we continue loading in the kernel's bootstrap code in C
  call   _start32

GLOBAL(halt_32):
  cli
  hlt
  jmp    halt_32

# Prints null terminated string %si direct to VGA memory
# First byte pointed to is the text attribute byte to use
GLOBAL(print_32):
  movl   $0xb8000,%edi    # text video memory address
  addl   (print_32_line_no),%edi
 print_loop_32:
  lodsb
  cmp    $'\n',%al
  je     crlf
  stosw
  or     %al,%al
  jne    print_loop_32
new_line:
  movl   (print_32_line_no),%edi
  addl   $160,%edi
  movl   %edi,(print_32_line_no)   # move to the next line
  # Note: doesn't handle scrolling the screen so don't print more than 24 lines
  ret
crlf:
  call   new_line
  jmp    print_32
GLOBAL(print_32_line_no):
  .long 320    # Text position

GLOBAL(a20_enabled):
  movl   $0x000500,%esi   # Hopefully isn't an address that matters, we will restore it
  movl   $0x100500,%edi   # %esi + 1MB  (if not A20 enabled, memory access will wrap to other address)
  xorl   %eax,%eax
  movb   (%esi),%cl       # Save original values
  movb   (%edi),%ch
  movb   $0x00,(%esi)     # Set to something
  movb   $0xFF,(%edi)     # If A20 not enabled, this write will write to 0x0000500
  movb   (%esi),%al       # When we read this, if A20 wasn't enabled we'll get either 0xFF, or 0x00
  movb   %cl,(%esi)       # Restore original values
  movb   %ch,(%edi)
  notb   %al
  ret                     # If A20 enabled, ax = 0xFF, else ax = 0x00

GLOBAL(bootloader_parameter_block_ptr):
  .long  0x0

GLOBAL(__cxa_atexit):
  xorl   %eax,%eax
  ret

GLOBAL(_dso_handle):
  .long  0x0

# All abnormal/unexpected paths redirect through to the panic handler
GLOBAL(_cxa_pure_virtual):
GLOBAL(start):
  nop
GLOBAL(k_panic):
  pushfl
  cli
  pushal
  pushl  %ds
  pushl  %es
  pushl  %fs
  pushl  %gs
  pushl  %cs
  call   _panic_helper    # We use call instead of jmp to push eip on to the stack
  loop_hlt:               # We don't expect it to return, but in case it does we halt
  hlt
  jmp    loop_hlt
  ret

GLOBAL(k_setjmp):
  pushfl
  cli
  pushl  %eax
  pushl  %ebx
  movl   0x10(%esp),%ebx # Copy param in to ebx (which is our dest jmpbuf_t)
  movl   %eax,0x00(%ebx) #  0 <- eax
  movl   %ecx,0x08(%ebx) #  8 <- ecx
  movl   %edx,0x0C(%ebx) # 12 <- edx
  movl   %esi,0x10(%ebx) # 16 <- esi
  movl   %edi,0x14(%ebx) # 20 <- edi
  movl   %ebp,0x18(%ebx) # 24 <- ebp
  movl   %esp,%eax
  addl   $0x14,%eax
  movl   %eax,0x1C(%ebx) # 28 <- esp
  movl   0x0C(%esp),%eax # Copy return address to eax
  movl   %eax,0x20(%ebx) # 32 <- eip
  movl   0x08(%esp),%eax # Copy flags saved to stack to eax
  movl   %eax,0x24(%ebx) # 36 <- flags
  movw   %cs,0x28(%ebx)  # 40 <- cs
  movw   %ds,0x2A(%ebx)  # 42 <- ds
  movw   %es,0x2C(%ebx)  # 44 <- es
  movw   %fs,0x2E(%ebx)  # 46 <- fs
  movw   %gs,0x30(%ebx)  # 48 <- gs
  movw   %ss,0x32(%ebx)  # 50 <- ss
  movl   %ebx,%eax
  popl   %ebx
  movl   %ebx,0x04(%eax) #  4 <- ebx
  popl   %eax
  popfl
  ret

GLOBAL(k_longjmp):
  pushfl
  cli
  pushl  %eax
  pushl  %ebx
  movl   0x10(%esp),%ebx # Copy param in to ebx (which is our source jmpbuf_t)
  movl   0x00(%ebx),%eax #  0 -> eax
  movl   0x08(%ebx),%ecx #  8 -> ecx
  movl   0x0C(%ebx),%edx # 12 -> edx
  movl   0x10(%ebx),%esi # 16 -> esi
  movl   0x14(%ebx),%edi # 20 -> edi
  movl   0x18(%ebx),%ebp # 24 -> ebp
  # TODO: setting the registers below has to be done correctly, perhaps
  #       need to prepare a stack with a return address and then swap the stack
  #       pointer and do a ret to have it jump to the saved eip
  movl   0x1C(%ebx),%eax # 28 -> esp
  movl   0x20(%ebx),%eax # 32 -> eip
  movl   0x24(%ebx),%eax # 36 -> flags
  movw   0x28(%ebx),%cs  # 40 -> cs
  movw   0x2A(%ebx),%ds  # 42 -> ds
  movw   0x2C(%ebx),%es  # 44 -> es
  movw   0x2E(%ebx),%fs  # 46 -> fs
  movw   0x30(%ebx),%gs  # 48 -> gs
  movw   0x32(%ebx),%ss  # 50 -> ss
  movl   %ebx,%eax
  popl   %ebx
  movl   %ebx,0x04(%eax) #  4 -> ebx
  popl   %eax
  popfl
  ret

gdt_msg:
  .asciz "Global Descriptor Table"

# Global descriptor table
GLOBAL(gdt_32):
  # Description of entry:
  #
  #   P - present, D - desc priv lvl / ring, C - 0=sys,1=code/data, T - seg type
  #   G - 4k granularity, B - seg size 0=16bit,1=32bit, L - 64bit code, A - system can use this bit
  #   S - seg limit bits 16-19
  #
  #      seg limit   base 0-15 16-23     PD C T      GBLA  S  base(24-31)
  #      |--------|  |--------|-----|    |/\|/--\    |||||--|  |--|
  #      0xff, 0xff, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Example Descriptor
  #
  .byte  0x00, 0x00, 0x00, 0x00, 0x00, 0b00000000, 0b00000000, 0x00  # Null   Descriptor
  .byte  0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00  # Code32 Descriptor
  .byte  0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10010010, 0b11001111, 0x00  # Data32 Descriptor
  .byte  0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10011010, 0b00001111, 0x00  # Code16 Descriptor
  .byte  0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10010010, 0b00001111, 0x00  # Data16 Descriptor
# .byte  0x68, 0x00, 0x00, 0x00, 0x00, 0b10001001, 0b11001111, 0x00  # TSS    Descriptor

  #
  # Same but only in hexidecimal for reference:
  #
  #  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Null   Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00  # Code32 Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00  # Data32 Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0x0F, 0x00  # Code16 Descriptor
  #  .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0x0F, 0x00  # Data16 Descriptor
  #  .byte 0x68, 0x00, 0x00, 0x00, 0x00, 0x89, 0xCF, 0x00  # TSS    Descriptor

# Magic value the BIOS looks for to tell if it is a valid MBR (last 2 bytes of the 1st disk sector)
.org 0x8000 - 2
boot_signature:
  .word  0x55AA

.text
.code16

# Entry point for the non-primary multi-core processors
.org 0x8000

GLOBAL(enter_smp_16):
  # call instead of jmp helps with setting up a call-stack
  pushw  $0x0         # fake it being a 32-bit return address
  call   enable_32

  # Reload Code Segment and Instruction Pointer (switches to 32-bit mode)
  calll  $0x0008,$protected_mode_smp_entry_32

GLOBAL(enable_32):
  # Enable A20 Gate (needed if plan to use >1MB of RAM)
  movw   $0x2401,%ax
  int    $0x15

  # Load Global Descriptor Table
  lgdtl  gdt32ptr

  # Mask the IRQs
  #movb  $0x00,%al
	#outb  %al,$0x21
  #movb  $0x00,%al
	#outb  %al,$0xA1

  # Enable 32-bit Protected Mode (set bit 0 in CR0), paging not enabled yet
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
  ret

# Code switches to 32-bit code here
.code32
protected_mode_smp_entry_32:
  movl   $smp_starting_msg,%esi
  movb   $0x0A,%ah
  call   print_32
  jmp    halt_32

smp_starting_msg:
  .ascii "Starting Core "
GLOBAL(smp_core_char1):
  .byte  '0'
GLOBAL(smp_core_char2):
  .byte  '0', 0

.section init_array_section
GLOBAL(init_array_start):
   # The compiler adds pointers to constructor functions to call in this section

.section init_early_section
GLOBAL(init_early_start):
   # Manually add custom init function pointers we want called before start32 to this section
