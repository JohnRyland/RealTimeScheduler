/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

struct keyboard_vtable_t
{
  bool (*key_pressed)();
  int  (*get_char)();
};
