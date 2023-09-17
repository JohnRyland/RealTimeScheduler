/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "timer.h"
#include "runtime.h"

// Current text position
static int curX;
static int curY;

unsigned random(unsigned)
{
  // TODO
  return 0;
}

void clrscr()
{
  memset((void*)0xB8000, 0, 0x10000);
}

void gotoxy(unsigned x, unsigned y)
{
  curX = x;
  curY = y;
}

void textmode(int mode)
{
  // do nothing
}

int getch()
{
  int key;
  asm (
    "AGAIN: \n"
    "  inb $0x64,%%al   # get the status \n"
    "  test $0x1,%%al  # check output buffer \n"
    "  jz NOKEY \n"
    "  testb $0x20,%%al # check if it is a PS2Mouse-byte \n"
    "  jnz NOKEY \n"
    "  inb $0x60,%%al   # get the key \n"
    "  jmp GOTKEY \n"
    "NOKEY: \n"
    "  jmp AGAIN \n"
    "GOTKEY: \n"
    "  " : "=r" (key));
  return key;
}

void putch(char ch)
{
  curX++;
  if (ch == '\n')
    curX = 0, curY++;
  else
    ((short*)0xB8000)[curY*80 + curX - 1] = 0x0700 | (unsigned)((unsigned char)ch);
}


bool kbhit()
{
  // TODO
  // checks if a key waiting
  // getch();
  return false;
}

void puts2(const char* str)
{
  while (*str)
    putch(*str++);
  putch('\n');
}


void init_timer_driver()
{
  // TODO
}

void delay(ticks_t number_of_ticks, tick_t deadline)
{
  // TODO
}

tick_t current_tick()
{
  // TODO
  return 0;
}

void set_current_tick(tick_t tick)
{
  // TODO
}

/*
struct timer_driver_t
{
  const char* name;
  bool (*install_preemptor)(tick_t event_time, preemptor_t user_func, void* user_data);
  bool (*uninstall_preemptor)();
  void (*enable)();
  void (*disable)();
  void (*suspend)();
  void (*resume)();
  void (*speed_up)();
  void (*slow_down)();
  //void (*set_speed)(unsigned int rate);
};
*/

timer_driver_t timer;

