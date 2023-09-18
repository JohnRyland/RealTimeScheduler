/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "constants.h"

// Some bogo delay value
#define  DELAY  5000000

extern int main();

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

void halt()
{
  for (;;)
    ;
}

extern "C"
void _start32()
{
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
  main();
  // Never return
  halt();
}

extern "C"
void start32()
{
  // Mach-O compiled version comes in via this function
  ((char*)VGA_TEXT_BASE)[0] = '.'; // debug - outputs something without using pointers to data
  _start32();
}
