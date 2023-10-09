/*
  RTOS Runtime
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "runtime/memory.h"

// Safer versions

struct mem
{
  void*   ptr;
  size_t  size;
};

template <size_t size>
struct memT
{
  void*   ptr;
};

using page = memT<4096>;

struct array
{
  mem     data;
  size_t  el_count;
  size_t  el_size;
};

void mem_set(mem dst, int val)
{
  uint8_t* dstc = (uint8_t*)dst.ptr;
  for (size_t i = 0; i < dst.size; ++i)
    dstc[i] = val;
}

bool mem_move(mem dst, const mem src)
{
  if (dst.size < src.size)
    return false;
  uint8_t* dstc = (uint8_t*)dst.ptr;
  uint8_t* srcc = (uint8_t*)src.ptr;
  if (dstc < srcc)
    for (size_t i = 0; i < src.size; ++i)
      dstc[i] = srcc[i];
  else
    for (size_t i = src.size-1; i >= 0; --i)
      dstc[i] = srcc[i];
  return true;
}

bool mem_cpy(mem dst, const mem src)
{
  return mem_move(dst, src);
}

int mem_cmp(const mem dst, const mem src)
{
  if ((dst.ptr == src.ptr) || (!dst.size && !src.size))
    return 0;
  size_t cmp_len = (dst.size < src.size) ? dst.size : src.size;
  uint8_t* dstc = (uint8_t*)dst.ptr;
  uint8_t* srcc = (uint8_t*)src.ptr;
  for (size_t i = 0; i < cmp_len; ++i)
  {
    if (dstc[i] < srcc[i])
      return -1;
    else if (dstc[i] > srcc[i])
      return 1;
  }
  if (dst.size < src.size)
    return -1;
  else if (dst.size > src.size)
    return 1;
  return 0;
}

bool mem_swap(mem first, mem second)
{
  if (first.size != second.size)
    return false;
  if (first.ptr == second.ptr)
    return true;
  uint8_t* left = (uint8_t*)first.ptr;
  uint8_t* right = (uint8_t*)second.ptr;
  for (size_t i = 0; i < first.size; ++i)
  {
    uint8_t tmp = left[i];
    left[i] = right[i];
    right[i] = tmp;
  }
  return true;
}



/*
  TODO: add some safety by encapsulating memory pointers with bounds.
  something like a min of:

    struct mem
    {
      void*   ptr;
      size_t  size;
    };
  
  perhaps optionally some alignment and element size members.

  Then everywhere void* is used, replace with ref to mem.
*/

void* mem_set(void* dst, int val, size_t len)
{
  uint8_t* dstc = (uint8_t*)dst;
  for (size_t i = 0; i < len; ++i)
    dstc[i] = val;
  return dst;
}

void* mem_move(void* dst, const void* src, size_t len)
{
  uint8_t* dstc = (uint8_t*)dst;
  uint8_t* srcc = (uint8_t*)src;
  if (dst < src)
    for (size_t i = 0; i < len; ++i)
      dstc[i] = srcc[i];
  else
    for (size_t i = len-1; i >= 0; --i)
      dstc[i] = srcc[i];
  return dst;
}

void* mem_cpy(void* dst, const void* src, size_t len)
{
  return mem_move(dst, src, len);
}

int mem_cmp(const void* dst, const void* src, size_t len)
{
  if (dst == src || !len)
    return 0;
  uint8_t* dstc = (uint8_t*)dst;
  uint8_t* srcc = (uint8_t*)src;
  for (size_t i = 0; i < len; ++i)
  {
    if (dstc[i] < srcc[i])
      return -1;
    else if (dstc[i] > srcc[i])
      return 1;
  }
  return 0;
}

void mem_swap(void* first, void* second, size_t len)
{
  if (first == second || !len)
    return;
  uint8_t* left = (uint8_t*)first;
  uint8_t* right = (uint8_t*)second;
  for (size_t i = 0; i < len; ++i)
  {
    uint8_t tmp = left[i];
    left[i] = right[i];
    right[i] = tmp;
  }
}


#if defined(_MACOS) || defined(_LINUX) || defined(_WIN32) || defined(_MSDOS)

// Hosted environment - we can assume we have a c library

#else

// We don't actually want to be calling these directly, but
// these are needed because the compiler inserts calls to these
// in the generated code when it detects a loop which could be
// replaced with a call to these.

extern "C"
{

void* memset(void* dst, int val, size_t len)
{
  return mem_set(dst, val, len);
}

void* memmove(void* dst, const void* src, size_t len)
{
  return mem_move(dst, src, len);
}

void* memcpy(void* dst, const void* src, size_t len)
{
  return mem_cpy(dst, src, len);
}

int memcmp(const void* dst, const void* src, size_t len)
{
  return mem_cmp(dst, src, len);
}

};

#endif
