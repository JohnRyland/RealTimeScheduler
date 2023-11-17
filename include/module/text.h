/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

struct text_display_t;

struct text_display_vtable_t
{
  void (*initialize)(text_display_t*);
  uint32_t (*width)(text_display_t*);
  uint32_t (*height)(text_display_t*);
  void (*goto_xy)(text_display_t*, uint32_t x, uint32_t y);
  void (*put_char)(text_display_t*, uint8_t attrib, char ch);
  void (*put_string)(text_display_t*, uint8_t attrib, const char* str);
  void (*clear)(text_display_t*);
};
