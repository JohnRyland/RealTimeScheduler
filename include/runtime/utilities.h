/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

extern "C"
{
  [[noreturn]]
  void halt();

  [[noreturn]]
  void exit(int);

  [[ noreturn ]]
  void critical_error(const char *error_message);

  // output helpers (avoids using printf)
  void print_str(const char* str);

  void print_int(int val);

  void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));

  unsigned random(unsigned x);
}
