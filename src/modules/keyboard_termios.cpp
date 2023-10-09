/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_KEYBOARD_TERMIOS

#include "module/keyboard.h"
#include "module_manager.h"

#include <cstdio>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <termios.h>

static
void enable_raw_mode()
{
  termios term;
  tcgetattr(0, &term);
  term.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO)); // Disable echo as well
  tcsetattr(0, TCSANOW, &term);
}

static
void disable_raw_mode()
{
  termios term;
  tcgetattr(0, &term);
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(0, TCSANOW, &term);
}

static
bool kbhit()
{
  enable_raw_mode();
  int byteswaiting;
  ioctl(0, FIONREAD, &byteswaiting);
  disable_raw_mode();
  return byteswaiting > 0;
}

static
int getch()
{
  enable_raw_mode();
  int ch = getchar();
  disable_raw_mode();
  return ch;
}

static
keyboard_vtable_t termios_vtable =
{
  .key_pressed = kbhit,
  .get_char    = getch,
};

static
module_t termios_driver = 
{
  .type    = module_class::KEYBOARD_DRIVER,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "termios" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &termios_vtable,
};

void register_keyboard_termios_driver()
{
  module_register(termios_driver);
}

#endif // ENABLE_KEYBOARD_TERMIOS
