/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_KEYBOARD_WIN32

#include "module/keyboard.h"
#include "module_manager.h"

static
void initialize()
{
}

static
bool key_pressed()
{
  return false;
}

static
int get_char()
{
  return 'x';
}

keyboard_vtable_t keyboard_win32_vtable
{
  .initialize = initialize,
  .key_pressed = key_pressed,
  .get_char = get_char,
};

static
module_t keyboard_win32_driver = 
{
  .type    = module_class::KEYBOARD_DRIVER,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "keyboard_win32" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &keyboard_win32_vtable,
};

void register_keyboard_win32_driver()
{
  module_register(keyboard_win32_driver);
}

#endif // ENABLE_KEYBOARD_WIN32
