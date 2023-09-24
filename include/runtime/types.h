/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "stdint.h"

/*
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef unsigned long       size_t;

static_assert(sizeof(int8_t)   == 1, "Unexpected int sizes");
static_assert(sizeof(int16_t)  == 2, "Unexpected int sizes");
static_assert(sizeof(int32_t)  == 4, "Unexpected int sizes");
static_assert(sizeof(int64_t)  == 8, "Unexpected int sizes");
static_assert(sizeof(uint8_t)  == 1, "Unexpected int sizes");
static_assert(sizeof(uint16_t) == 2, "Unexpected int sizes");
static_assert(sizeof(uint32_t) == 4, "Unexpected int sizes");
static_assert(sizeof(uint64_t) == 8, "Unexpected int sizes");
static_assert(sizeof(size_t)   == 8, "Unexpected int sizes");
*/

typedef unsigned long       size_t;

struct short_name
{
  char const name[16];
};
