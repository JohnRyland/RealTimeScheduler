/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_KEYBOARD_DOS

#include "module/keyboard.h"
#include "module_manager.h"

#include "conio.h"

static
void initialize()
{
}

static
bool key_pressed()
{
  return kbhit();
}

static
int get_char()
{
  return getch();
}

keyboard_vtable_t keyboard_dos_vtable
{
  .initialize = initialize,
  .key_pressed = key_pressed,
  .get_char = get_char,
};

static
module_t keyboard_dos_driver = 
{
  .type    = module_class::KEYBOARD_DRIVER,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "keyboard_dos" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &keyboard_dos_vtable,
};

void register_keyboard_dos_driver()
{
  module_register(keyboard_dos_driver);
}

#endif // ENABLE_KEYBOARD_DOS
