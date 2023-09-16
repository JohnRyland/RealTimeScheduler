/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

// Some bogo delay value
#define  DELAY  10000000

// Text attribute values
#define  BOLD   0x0F
#define  NORMAL 0x08
#define  RED    0x0C
#define  GREEN  0x0A

// Current line
int line;

void println(char attrib, char* str)
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

void clrscr()
{
  volatile char* videoMemory = (char*)0xB8000;
  for (int i = 0; i < 65535; ++i)
    videoMemory[i] = 0;
}

int udelay(long x)
{
  volatile char* videoMemory = (char*)0xB8000;
  int t = 0;
  for (int i = 0; i < x; ++i)
    t += videoMemory[i & 0xFFFF];
  return t;
}

void start32()
{
  volatile char* videoMemory = (char*)0xB8000;
  videoMemory[0] = '.'; // debug - outputs something without using pointers to data
  line = 1760; // Display this after the bootloader's text
  println(GREEN, "[X] Entered C start");
  udelay(DELAY);
  clrscr();
  line = 0;
  println(GREEN, "[X] Starting");

  // Never return
  for (;;)
    ;
}


