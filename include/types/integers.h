/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../runtime/compatibility.h"

// These sized integer types help a lot with
// dealing with fixed hardware data structures
#if 0 // __cplusplus >= 201103L
#  include <cstdint>
#  include <cstddef>
#else
  typedef signed char         int8_t;
  typedef signed short        int16_t;
  typedef signed int          int32_t;
  typedef signed long long    int64_t;
  typedef unsigned char       uint8_t;
  typedef unsigned short      uint16_t;
  typedef unsigned int        uint32_t;
  typedef unsigned long long  uint64_t;
  typedef unsigned long       size_t;
#endif

// These are what I think are sane values on most platforms.
// If these static asserts fail then the fix is to correct
// the typedefs above as applicable for the given platform.
// There is an assumption that the given platform's compiler
// is able to support generating code for 64-bit values and
// it might be that some more low-end platforms don't support
// this, so I think to make the OS scalable that the code
// might need to try to handle this case and be able to gracefully
// fallback, yet still be able to support 64-bit platforms.
STATIC_ASSERT(sizeof(int8_t)   == 1);
STATIC_ASSERT(sizeof(int16_t)  == 2);
STATIC_ASSERT(sizeof(int32_t)  == 4);
STATIC_ASSERT(sizeof(int64_t)  == 8);
STATIC_ASSERT(sizeof(uint8_t)  == 1);
STATIC_ASSERT(sizeof(uint16_t) == 2);
STATIC_ASSERT(sizeof(uint32_t) == 4);
STATIC_ASSERT(sizeof(uint64_t) == 8);

// Absolute size of size_t depends if a 32-bit build or 64-bit build,
// but we assume that it is the same size as the size of a pointer.
// If that is not true on a given platform then there could be places
// where the code will not behave as expected so all usages of size_t
// would need to be checked.
STATIC_ASSERT(sizeof(size_t) == sizeof(void*));
