/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

enum class depth_t : uint8_t
{
  DEPTH_16,
  DEPTH_24,
  DEPTH_32,
};

struct graphics_vtable_t
{
  bool (*initialize)(uint32_t width, uint32_t height, depth_t depth);
  uint32_t (*width)();
  uint32_t (*height)();
  depth_t (*depth)();
  void (*set_pixel)(uint32_t x, uint32_t y, uint32_t val);
};
