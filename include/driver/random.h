/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

struct random_driver_t
{
  int (*random)(int min, int max);
};
