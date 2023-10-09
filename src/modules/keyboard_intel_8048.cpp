/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_KEYBOARD_INTEL_8048  // PPI

#include "module/keyboard.h"
#include "module_manager.h"

#include "constants.h"
#include "x86.h"

//   - Programmable Peripheral Interface (8042)
#        define   PPI_DATA            0x60
#        define   PPI_COMMAND         0x64
#        define   PPI_STATUS          0x64

static
void initialize()
{
}

static
bool key_pressed()
{
  // check if a key is waiting
  int status = inportb(PPI_STATUS);
  if (!(status & 1))    // check output buffer
    return false;
  if ((status & 0x20))  // check if PS2 mouse byte
    return false;
  return true;
}

static
int get_char()
{
    while (!key_pressed())
    /* wait */;
  unsigned int ch = (unsigned char)inportb(PPI_DATA); // read the key that is pressed

  // Rough keymap
  static const char keymap[] = " \0331234567890-=\b\tqwertyuiop[]\r asdfghjkl;'~ \nzxcvbnm,./ **               789-456+1230 ";

  bool press = ch <= 127;
  unsigned int mapped = (press) ? ch : (ch - 128);
  if (mapped < sizeof(keymap))
    mapped = keymap[mapped];

  if (press)
    return mapped;

  return -1;
}

keyboard_vtable_t keyboard_intel_8048_vtable
{
  .initialize = initialize,
  .key_pressed = key_pressed,
  .get_char = get_char,
};

static
module_t keyboard_intel_8048_driver = 
{
  .type    = module_class::KEYBOARD_DRIVER,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "key_intel_8048" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &keyboard_intel_8048_vtable,
};

void register_keyboard_intel_8048_driver()
{
  module_register(keyboard_intel_8048_driver);
}

#endif // ENABLE_KEYBOARD_INTEL_8048

