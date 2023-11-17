/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

struct keyboard_vtable_t
{
  void (*initialize)();
  bool (*key_pressed)();
  int  (*get_char)();
};
