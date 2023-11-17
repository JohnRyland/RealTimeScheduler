/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "compatibility.h"
#include "types/modules.h"

#include "module/text.h"
#include "module/keyboard.h"
#include "module_manager.h"
#include "kernel/debug_logger.h"

#include <conio.h>


// Hosted environments
#if defined(_MACOS) || defined(_LINUX) || defined(_WIN32) || defined(_MSDOS)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern int __main(int argc, const char* argv[]);

int main()
{
  int argc = 2;
  const char* argv[] = { "hosted", "hosted" };
  return __main(argc, argv);
}

void initialize_banner()
{
}

void initialize_vesa()
{
}

void initialize_screen()
{
}

void k_log_early(log_level /*level*/, const char* str)
{
  puts(str);
}

extern "C"
NO_RETURN
void k_panic()
{
  puts(" *** PANIC *** \n");
  exit(0);
}

void k_power_off()
{
  ::exit(0);
}

void k_sleep(int seconds)
{
  sleep(seconds);
}

#endif

#ifdef _I386

#include "kernel/bios.h"

void k_power_off()
{
  // APM - power-off
  call_real_mode_interrupt(0x15, 0x5307, 0x0001, 0x0003);
}

void k_msleep(int milliseconds)
{
  uint64_t now;
  asm volatile ( "rdtsc" : "=A"(now) );
  uint64_t end = now + 3000000ULL * milliseconds;
  while (now < end)
    asm volatile ( "rdtsc" : "=A"(now) );
}

void k_sleep(int seconds)
{
  uint64_t now;
  asm volatile ( "rdtsc" : "=A"(now) );
  uint64_t end = now + 3000000000ULL * seconds;
  while (now < end)
    asm volatile ( "rdtsc" : "=A"(now) );

//  uint32_t delayInTimerUnits = seconds * 0x3FF;
//  call_real_mode_interrupt(0x15, 0x8600, 0x0000, 0, 0x3ff);
//  call_real_mode_interrupt(0x15, 0x8600, 0x0000, delayInTimerUnits >> 16, delayInTimerUnits & 0xFFFF);
//    call_real_mode_interrupt(0x15, 0x8600, 0x0000, 0x000F, 0x4240);  // from code, this is 1000000 in hex, it looks to assume microseconds
//  for (int i = 0; i < seconds; i++)
//    call_real_mode_interrupt(0x15, 0x8600, 0x0000, 0x0000, 0x03FF);
}

#endif

#ifdef _MSDOS

// TC/DOS conio library functions
bool kbhit();
int random(int x);
void clrscr();
void gotoxy(int x, int y);
void textmode(int);
int getch();
void putch(unsigned char ch);

void puts2(const char* str)
{
  while (*str)
  {
    putch(*str);
    ++str;
  }
  printf("\n");
}

#elif defined(_MACOS) || defined(_LINUX) || defined(_WIN32) || defined(_I386)

struct conio_data
{
  bool                    init;
  text_display_t*         text_display_data;
  text_display_vtable_t*  text_display_functions;
  keyboard_vtable_t*      keyboard_functions;
};

//static
conio_data state;

void initialize_conio()
{
  state.init = false;
  module_t const* keyboard_module = find_module_by_class(module_class::KEYBOARD_DRIVER);
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  if (keyboard_module && text_display_module)
  {
    state.keyboard_functions = (keyboard_vtable_t*)keyboard_module->vtable;
    state.text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
    state.text_display_data = (text_display_t*)text_display_module->instance;
    state.init = true;
  }
}

extern void goto_xy(unsigned x, unsigned y);
extern void clear_screen();
extern void log_char(char ch);

extern "C"
{

bool kbhit()
{
  if (state.init == false)
    return true;
  return state.keyboard_functions->key_pressed();
}

void clrscr()
{
  if (state.init == false)
    return;
  clear_screen();
  // state.text_display_functions->clear(state.text_display_data);
}

void gotoxy(unsigned x, unsigned y)
{
  if (state.init == false)
    return;
  goto_xy(x, y);
  // state.text_display_functions->goto_xy(state.text_display_data, x, y);
}

void textmode(int)
{
  if (state.init == false)
    return;
  state.text_display_functions->initialize(state.text_display_data);
}

int getch()
{
  if (state.init == false)
    return 0;
  return state.keyboard_functions->get_char();
}

void putch(char ch)
{
  if (state.init == false)
    return;
  log_char(ch);
  // state.text_display_functions->put_char(state.text_display_data, 0x08, ch);
}

void puts2(const char* str)
{
  if (state.init == false)
    return;
  state.text_display_functions->put_string(state.text_display_data, 0x08, str);
}

}; // extern "C"

#endif
