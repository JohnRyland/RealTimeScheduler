/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "constants.h"
#include "helpers.h"
#include "x86.h"

// Some bogo delay value
#define  DELAY  5000000

static int line;

void println(char attrib, const char* str)
{
  volatile char* videoMemory = (char*)0xB8000;
  for (int i = 0; *str; i+=2)
  {
    videoMemory[i + line + 0] = *str;
    videoMemory[i + line + 1] = attrib;
    str++;
  }
  line += 160;
}

int udelay(long x)
{
  volatile char* videoMemory = (char*)0xB8000;
  int t = 0;
  for (int i = 0; i < x; ++i)
    t += videoMemory[i & 0xFFFF];
  return t;
}

void setup_initial_pagetables()
{
  write_cr3(0x1000000);
  for (int i = 0; i < 0xe000; ++i)
    ((uint32_t*)0x1000000)[i] = 0;
  uint32_t* page_map = (uint32_t*)read_cr3();
  page_map[0*4096] = 0x01002000;   // page-map level 4
  page_map[1*4096] = 0x01004000;   // page-directory pointer table

  page_map[2*4096+8*0] = 0x01006000;   // page-directory 1st 2MB
  page_map[2*4096+8*1] = 0x01008000; // page-directory 2nd 2MB

  // page-table for mapping the 1st 4MB
  for (int i = 0; i < 1024; ++i)
    page_map[3*4096+8*i] = 0x01000 * i;
}

// This is written in assembler and has a bunch of trampolines
extern "C" void load_idt();

void setup_interrupts_table()
{
  load_idt();
}

void remap_irqs()
{
  outportb(0x20, 0x11);  // starts the initialization sequence (in cascade mode)
  outportb(0xA0, 0x11);

  outportb(0x21, 0x20);  // set the IVT entry for the IRQs
  outportb(0xA1, 0x28);

  outportb(0x21, 0x04);  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  outportb(0xA1, 0x02);  // ICW3: tell Slave PIC its cascade identity (0000 0010)

  outportb(0x21, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  outportb(0xA1, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
}

void set_irq_mask(uint16_t irqMask)
{
  outportb(0x21, irqMask & 0xff);
  outportb(0xA1, irqMask >> 8);
}

void setup_interrupts()
{
  load_idt();
  remap_irqs();
  set_irq_mask(0xfffc); // unmask the timer and keyboard interrupts
}

extern "C"
void _start32()
{
  setup_initial_pagetables();
  setup_interrupts();

  // relocate the stack - we can't really put this in a function as there is no way to ret
  asm volatile ( "mov %0, %%esp" : : "a"(0x01008000) ); // (Second 2 MiB)

  ((char*)VGA_TEXT_BASE)[2] = '.'; // debug - outputs something without using pointers to data
  line = 1760; // Display this after the bootloader's text
  println(GREEN, "[X] Entered C start");
  gotoxy(0, 12);
  puts2("[X] Conio");
  udelay(DELAY);
  clrscr();
  line = 0;
  println(NORMAL, "Starting...");
  udelay(DELAY);

  // Start the scheduler
  int ret = main(0, nullptr);

  // Never return
  exit(ret);
}

extern "C"
void start32()
{
  // Mach-O compiled version comes in via this function
  ((char*)VGA_TEXT_BASE)[0] = '.'; // debug - outputs something without using pointers to data
  _start32();
}

