#
# x86 Interrupt Setup
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

#include "macros.S"

.text
.code32

.equ idt_count_1, 23
.equ idt_count_2, 25
.equ idt_count,   idt_count_1+idt_count_2

# Load Interrupt Descriptor Table
GLOBAL(load_idt):
  pushal
  movl   $isr_entry_1,%eax
  movl   $idt,%esi
  movl   $0xEB006A60,%ebx   # this is the machine code of:   pushal ; pushl $0 ; jmp ...
  movl   $idt_count_1,%ecx
  call   interrupt_descriptor_table_entry_load_loop
  addl   $2,%eax
  movl   $idt_count_2,%ecx
  call   interrupt_descriptor_table_entry_load_loop
  lidt   idt32ptr
  popal
  ret

  #
  # Interrupt descriptor table entry format
  #
  #   P = present
  #   DPL = 2-bit ring level
  #   GT = gate type ( b0101 = task, b1110 = interrupt, b1111 = trap)
  #
  #                                    DPL GT reserved
  #                                   P||0|--||------|
  #        offset 0-15      segment       flags       offset 16-31
  #     |----------------| |-------| |---------------| |-----|
  #.word       0x0000,       0x0000, 0b0000000000000000, 0x0000

interrupt_descriptor_table_entry_load_loop:
  movw   %ax,(%esi)
  pushl  %eax
  shrl   $16,%eax
  movw   %cs,2(%esi)
  movw   $0b1110111000000000,4(%esi)  # see entry format above
  movw   %ax,6(%esi)
  popl   %eax
  addl   $8,%esi
  movl   %ebx,(%eax)      # %ebx has machine code of:  pushal ; pushl $INTERRUPT_NUM ; jmp double_jump/generic_isr
  lea    -5(%ecx,%ecx,4),%edx
  movb   %dl,4(%eax)      # we now are putting in the relative jump address to double_jump or generic_isr
  addl   $0x10000,%ebx
  addl   $5,%eax
  loop   interrupt_descriptor_table_entry_load_loop
  ret

# TODO: if can write the jmp instr, then could move this to the bss
isr_entry_1:
  .skip  idt_count_1*5
double_jump:
  jmp    generic_isr
  .skip  idt_count_2*5

generic_isr:
  cli    # disable interrupts
  cld
  call   _interrupt_handler  # Up to the handler check the In-Service-Register and send EOI as required
  popl   %eax
  popal
  iret

# Interrupt descriptor table size+pointer
idt32ptr:
  .word  idt_count*8 - 1
idt32ptr_2:
  .long  idt

.bss

# Interrupt descriptor table
idt:
  .skip  idt_count*8

.text
.code32

real_mode_idt:
	.word	 0x3FF    # newos uses 0x7FF! why the difference?
	.long	 0

GLOBAL(reg_buffer):
  .long  0,0,0,0,  0,0,0,0,  0,0,0,0,  0

GLOBAL(reboot):
  # Clear interrupts
  cli
  # Load the real mode IDT
  lidt   real_mode_idt
  # Reset / restart
	ljmpw	 $0xf000,$0xfff0   # I think I read code that uses "int $1" for reboot

.equ CODE16_SEL, 24
.equ DATA16_SEL, 32

GLOBAL(call_real_mode):
  # save state
  pushal
  movl   %esp,%ebx

  # Clear interrupts
  cli

  # Mask all IRQs
  xorl   %eax,%eax
  outb   %al,$0x21
  outb   %al,$0xA1

  # Load the real mode IDT
  lidt   real_mode_idt

protected_mode_32:

  # Set CS to the CODE16 entry in the GDT (we are still in PM but code will be 16-bit)
	ljmpl	 $CODE16_SEL, $protected_mode_16

.code16
protected_mode_16:

  # Point selectors at the DATA16 entry in the GDT
	movw	 $DATA16_SEL, %cx
	movw	 %cx,%ds
	movw	 %cx,%es
	movw	 %cx,%fs
	movw	 %cx,%gs
	movw	 %cx,%ss

  # Change the CPU control register state out of PM (and disable paging if it was enabled)
	movl	 %cr0,%eax
  andl   $0x7FFFFFFE,%eax
	movl	 %eax,%cr0

  # Do a long jump to enact the change to the PM state
  ljmpl  $0,$real_mode_16

.code16
real_mode_16:

  # Debug output to serial port
  mov    $10,%cx
send_dot2:
  mov    $0x03F8,%dx
  add    $5,%dx
wait_to_send2:
  inb    %dx,%al
  andb   $0x20,%al
  jz     wait_to_send2
  sub    $5,%dx

#  # Print a "+" to serial to debug and see that the we got here, also to measure how many real-mode calls we are making
#  mov    $'+',%al
#  outb   %al,%dx

  dec    %cx
  jnz    send_dot2

  # Setup stack for real mode
  xorw   %cx,%cx
	movw	 %cx,%ds
	movw	 %cx,%ss
  mov    $0x7c00,%sp

  # Save state
  pushal
  pushw  %es
  pushw  %ds

  # Setup registers with user parameters for the interrupt call
  movl   $reg_buffer,%edi
  movl   4(%edi),%ebx   # ebx
  movl   8(%edi),%ecx   # ecx
  movl   12(%edi),%edx  # edx
  movl   16(%edi),%esi  # esi
  pushl  0(%edi)        # eax
  pushl  20(%edi)       # edi
  pushw  24(%edi)       # es
  pushw  28(%edi)       # ds
  popw   %ds
  popw   %es
  popl   %edi
  popl   %eax

  # calling real mode BIOS/interrupt call
.byte    0xCD
GLOBAL(real_mode_sw_interrupt):
.byte    0x10

  # Get DS:DI point to our reg_buffer (and save DI)
  popw   %ds

#  pushl  %edi
#  movl   $reg_buffer,%edi

  # DS is clobbered

  # Save registers ES, DS and DI first before we clobber them
  pushl  %esi
  pushl  %eax
  pushw  %es
  movl   %edi,%esi
  movl   $reg_buffer,%edi
  movl   %esi,20(%edi)   # saving EDI
  xorl   %eax,%eax
  movw   %es,%ax
  movl   %eax,24(%edi)   # saving ES
  movw   %ds,%ax
  movl   %eax,28(%edi)   # saving DS
  popw   %es
  popl   %eax
  popl   %esi

  # Save the resulting register values back to the buffer
  movl   %eax,0(%edi)
  movl   %ebx,4(%edi)
  movl   %ecx,8(%edi)
  movl   %edx,12(%edi)
  movl   %esi,16(%edi)
  # movw   %ds,28(%esi)  # TODO: DS was clobbered, does any INT return via DS?

  # Restore rest of state
  popw   %es
  popal

  # Enable 32-bit Protected Mode (set bit 0 in CR0)
  movl   %cr0,%eax
  orl    $0x01,%eax    # TODO: this doesn't reenable paging if it was enabled
  movl   %eax,%cr0

  # Setup Segment Registers
  xorl   %eax,%eax
  movw   %ax,%fs
  movw   %ax,%gs
  movb   $0x10,%al
  movw   %ax,%ds
  movw   %ax,%es
  movw   %ax,%ss

  # Reload Code Segment and Instruction Pointer (switches to 32-bit mode)
  ljmpl  $0x0008,$reenter_protected_mode

.code32
reenter_protected_mode:
  lidt   idt32ptr
  movl   %ebx,%esp
  popal
  ret
