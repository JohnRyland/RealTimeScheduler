/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

extern "C"
{
  void* memset(void* dst, int val, size_t len);

  void* memmove(void* dst, const void* src, size_t len);

  void* memcpy(void* dst, const void* src, size_t len);
}
