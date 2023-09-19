/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "runtime.h"



extern "C"
{
void* memset(void* dst, int val, size_t len)
{
  char* dstc = (char*)dst;
  for (int i = 0; i < len; ++i)
    dstc[i] = val;
  return dst;
}

void* memmove(void* dst, const void* src, size_t len)
{
  char* dstc = (char*)dst;
  char* srcc = (char*)src;
  if (dst < src)
    for (int i = 0; i < len; ++i)
      dstc[i] = srcc[i];
  else
    for (int i = len-1; i >= 0; i--)
      dstc[i] = srcc[i];
  return dst;
}

void* memcpy(void* dst, const void* src, size_t len)
{
  return memmove(dst, src, len);
}

void exit(int)
{
  // TODO
  for (;;)
    ;
}

void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *))
{
  // TODO
}

}

