/*
 x86 OS Kernel Panic
 Copyright (C) 2023, John Ryland
 All rights reserved.
*/

#include "compatibility.h"
#include "runtime/utilities.h"
#include "types/symbol.h"
#include "kernel/debug_logger.h"
#include "kernel/exception_handler.h"
#include "runtime/conio.h"

static int col;
static int row;
static char attrib2;

static
void new_line()
{
  col = 0;
  row++;
  if (row > 49)
  {
    char* videoMemory = (char*)0xB8000;
    for (int i = 0; i < 160*49; ++i)
      videoMemory[i] = videoMemory[i+160];
    for (int i = 0; i < 160; ++i)
      videoMemory[i+160*49] = 0;
    row--;
  }
}

static
void add_char(char ch)
{
  char* videoMemory = (char*)0xB8000;
  videoMemory[row*160+col*2+0] = ch;
  videoMemory[row*160+col*2+1] = attrib2;
  col++;
  if (col >= 80)
    new_line();
}

static
void failsafe_print(const char* string)
{
  while (*string)
  {
    if (*string == '\n')
      new_line();
    else
      add_char(*string);
    string++;
  }
  new_line();
}

static
void failsafe_print_n(const char* string, int count)
{
  for (int i = 0; i < count; ++i)
    add_char(string[i]);
}

static
void stamp_hex32(char* dest, uint32_t val)
{
  static const char hex_digit[] = "0123456789ABCDEF";
  for (int i = 0; i < 8; ++i)
    dest[i] = hex_digit[(val >> ((7 - i)*4)) & 0xF];
}

static
void init()
{
  col = 0;
  row = 5;
  attrib2 = 0x07;
}

struct registers_t
{
  uint32_t flags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
  uint32_t cs;
};

struct jmpbuf_t
{
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t eip;
  uint32_t flags;
  uint16_t cs;
  uint16_t ds;
  uint16_t es;
  uint16_t fs;
  uint16_t gs;
  uint16_t ss;
};

extern "C"
void k_setjmp(jmpbuf_t& buf);

extern "C"
void k_longjmp(jmpbuf_t& buf);

NO_RETURN
void k_halt()
{
  // TODO: call cpu module
  for (;;)
    ;
}

extern void k_power_off();

NO_RETURN
void exit(int code)
{
  // If error happens early, ensure we see something
  k_log_early(ERROR, " *** EXIT *** ");

  // If happens later, give more details
  //clrscr();
  //gotoxy(0, 1);
  k_log_fmt(ERROR, " *** EXIT *** \n Exit code %i\n", code);

  k_power_off();
  k_halt();
}

extern "C"
const uint8_t symbol_map_base[32];

int compare_symbols(const void* a, const void* b)
{
  const symbol_entry* sym_a = (const symbol_entry*)a;
  const symbol_entry* sym_b = (const symbol_entry*)b;
  const symbol_entry* sym_c = &sym_b[1];
  if (sym_a->address <= sym_b->address)
    return -1;
  if (sym_a->address > sym_c->address)
    return 1;
  return 0;
}

void dump_callstack(uint32_t* stack_pointer)
{
  const symbol_table* symbol_map = (const symbol_table*)&symbol_map_base;
  const symbol_entry* symbol_entries = symbol_map->entries;
  const uint16_t symbol_entry_count = symbol_map->count;
  const char* symbol_map_strs = (const char*)symbol_entries + symbol_entry_count * sizeof(symbol_entry);
  uint32_t print_count = 0;

  while ((size_t)stack_pointer < 0x0208000)
  {
    uint32_t val = *stack_pointer;
    if (val > 0x7C00 && val < 0x10000)
    {
      static const char stack_value[] = "0x00000000 ";

      symbol_entry key = { val, 0 };
      const symbol_entry* ent = (const symbol_entry*)k_bsearch((void*)&key, symbol_entries, symbol_entry_count, sizeof(symbol_entry), compare_symbols);
      
      stamp_hex32((char*)&stack_value[2], (size_t)stack_pointer);
      failsafe_print_n(stack_value, 11);

      stamp_hex32((char*)&stack_value[2], val);
      failsafe_print_n(stack_value, 11);

      uint32_t len = ent[1].symbol_offset - ent[0].symbol_offset;
      if (len > 44)
        len = 44;
      failsafe_print_n(symbol_map_strs + ent[0].symbol_offset, len);
      new_line();

      ++print_count;
      if (print_count > 30)
        break;
    }
    
    stack_pointer++;
  }
}

void dump_registers(jmpbuf_t& /*registers*/)
{
}

void panic_helper_impl(int off)
{
  uint32_t flags;

  init();

  attrib2 = 0x0C;
  failsafe_print("\n\nKernel Panic!\n");
  
  attrib2 = 0x0F;
  failsafe_print("\nRegisters:\n\n");

  static const char reg_msg[] =
    "Flg: 0x00000000\n"
    "EAX: 0x00000000 ECX: 0x00000000 EDX: 0x00000000 EBX: 0x00000000\n"
    "ESP: 0x00000000 EBP: 0x00000000 ESI: 0x00000000 EDI: 0x00000000\n"
    "DS:  0x00000000 ES:  0x00000000 FS:  0x00000000 GS:  0x00000000\n"
    "CS:  0x00000000 EIP: 0x00000000\n"
    "\n"
    "Stack trace:\n"
    "  Address    Value      Symbol";

  char reg_msg_buf[sizeof(reg_msg)];
  for (size_t i = 0; i < sizeof(reg_msg); ++i)
    reg_msg_buf[i] = reg_msg[i];;
  //memcpy(reg_msg_buf, reg_msg, sizeof(reg_msg)); // problem? link error?

  for (int i = 0; i < 15; ++i)
    stamp_hex32((char*)&reg_msg_buf[7 + 16*i], (&flags)[off-i]);
  failsafe_print(reg_msg_buf);
  
  uint32_t* stack_pointer = &((&flags)[off-14]);
  dump_callstack(stack_pointer - 32);
}

extern "C"
void panic_helper()
{
  col = 0;
  row = 0;
  attrib2 = 0x0C;
  failsafe_print("MachO Build\n");
  panic_helper_impl(21);
}

extern "C"
void _panic_helper()
{
  col = 0;
  row = 0;
  attrib2 = 0x0C;
  failsafe_print("ELF Build\n");
  panic_helper_impl(37);
}

void k_fault_handler(fault_t fault)
{
  static const char* faults[] = {
    "Division by zero",
    "Single-step trap",
    "Non-maskable interrupt",
    "Debug breakpoint",
    "Arithmetic overflow",
    "Bounds range exceeded",
    "Invalid opcode",
    "Coprocessor not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid task state segment",
    "Segment not present",
    "Stack segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 floating point exception",
    "Alignment check",
    "Machine check",
    "SIMD floating point exception",
    "Virtualization exception",
    "Control protection exception",
    "Reserved (0x16)",
    "Reserved (0x17)",
    "Reserved (0x18)",
    "Reserved (0x19)",
    "Reserved (0x1A)",
    "Reserved (0x1B)",
    "Reserved (0x1C)",
    "Reserved (0x1D)",
    "Reserved (0x1E)",
    "Reserved (0x1F)",
  };

  static const size_t faults_count = sizeof(faults) / sizeof(faults[0]);

  const size_t fault_index = (size_t)fault % faults_count;
  
  // If fault happens early, ensure we see something
  k_log_early(CRITICAL, " *** CPU fault *** ");
  k_log_early(CRITICAL, faults[fault_index]);

  // If happens later, give more details
  clrscr();
  gotoxy(0, 1);
  k_log_fmt(CRITICAL, "\n *** CPU fault *** \n Fault number: %i \n Fault description: %s \n", fault, faults[fault_index]);

  k_panic();
  exit(-(int)fault);
}

NO_RETURN
void k_critical_error(int code, const char* fmt, ...)
{
  uint32_t stack_variable;

  // If error happens early, ensure we see something
  k_log_early(CRITICAL, " *** CRITICAL ERROR *** ");

  // If happens later, give more details
  clrscr();
  gotoxy(0, 1);

  //timer.disable();
  va_list ap;
  va_start(ap, fmt); 
  k_log_vfmt(CRITICAL, fmt, ap);
  k_log_fmt(CRITICAL, "*** CRITICAL ERROR ***\nError code: %i\n", code);

  dump_callstack(&stack_variable);
  //k_panic();

  k_halt();
  va_end(ap);
}


