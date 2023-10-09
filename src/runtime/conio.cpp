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
#include "kernel/exception_handler.h"
#include "kernel/debug_logger.h"

#include <conio.h>


// Hosted environments
#if defined(_MACOS) || defined(_LINUX) || defined(_WIN32) || defined(_MSDOS)

#include <stdio.h>

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
  text_display_t*         text_display_data;
  text_display_vtable_t*  text_display_functions;
  keyboard_vtable_t*      keyboard_functions;
};

static
conio_data state;

void conio_init()
{
  module_t const* keyboard_module = find_module_by_class(module_class::KEYBOARD_DRIVER);
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  state.keyboard_functions = (keyboard_vtable_t*)keyboard_module->vtable;
  state.text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
  state.text_display_data = (text_display_t*)text_display_module->instance;
}

extern "C"
{

bool kbhit()
{
  return state.keyboard_functions->key_pressed();
}

void clrscr()
{
  state.text_display_functions->clear(state.text_display_data);
}

void gotoxy(unsigned x, unsigned y)
{
  state.text_display_functions->goto_xy(state.text_display_data, x, y);
}

void textmode(int)
{
  state.text_display_functions->initialize(state.text_display_data);
}

int getch()
{
  return state.keyboard_functions->get_char();
}

void putch(char ch)
{
  state.text_display_functions->put_char(state.text_display_data, 0x08, ch);
}

void puts2(const char* str)
{
  state.text_display_functions->put_string(state.text_display_data, 0x08, str);
}

}; // extern "C"

#endif
