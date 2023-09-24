/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

extern "C"
{
  void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));

  [[noreturn]]
  void exit(int);
}
