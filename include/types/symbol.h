/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "integers.h"

struct symbol_entry
{
  uint32_t     address;
  uint32_t     symbol_offset;
};

// Kernel Symbol Table / Map
struct symbol_table
{
  uint16_t     magic;  // 0xDDCC
  uint16_t     count;
  symbol_entry entries[];
  // followed by string data
};
