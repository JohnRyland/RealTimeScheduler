.code32

.equ idt_count_1, 23
.equ idt_count_2, 25
.equ idt_count,   idt_count_1+idt_count_2

# Load Interrupt Descriptor Table
.globl load_idt
load_idt:
.globl _load_idt
_load_idt:
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

isr_entry_1:
  .skip idt_count_1*5
double_jump:
  jmp    generic_isr
  .skip idt_count_2*5

generic_isr:
  cli    # disable interrupts
  cld
  call   _interrupt_handler  # Up to the handler check the In-Service-Register and send EOI as required
  popl   %eax
  popal
  iret

# Interrupt descriptor table size+pointer
idt32ptr:
  .word idt_count*8 - 1
idt32ptr_2:
  .long idt

.bss

# Interrupt descriptor table
idt:
  .skip idt_count*8
