/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

enum class depth_t : uint8_t
{
  DEPTH_16,
  DEPTH_24,
  DEPTH_32,
};

struct graphics_display_t;

struct graphics_display_vtable_t
{
  bool (*initialize)(graphics_display_t*, uint32_t width, uint32_t height, depth_t depth);
  uint32_t (*width)(graphics_display_t*);
  uint32_t (*height)(graphics_display_t*);
  depth_t (*depth)(graphics_display_t*);
  void (*set_pixel)(graphics_display_t*, uint32_t x, uint32_t y, uint32_t val);
  // TODO: more funcs like bitblit, drawrect, line and more
};
