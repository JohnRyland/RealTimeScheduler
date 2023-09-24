/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

struct text_vtable_t
{
  uint32_t (*width)();
  uint32_t (*height)();
  void (*gotoxy)(uint32_t x, uint32_t y);
  void (*put_char)(uint8_t attrib, char ch);
  void (*put_string)(uint8_t attrib, const char* str);
  void (*clear)();
};
