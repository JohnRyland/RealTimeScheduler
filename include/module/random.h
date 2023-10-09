/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

struct random_device_vtable_t
{
  void (*initialize)();
  int (*random)(int min, int max);
};
