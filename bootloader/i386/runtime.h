/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

typedef int size_t;


extern "C"
{
void* memset(void* dst, int val, size_t len);
void* memmove(void* dst, const void* src, size_t len);
void* memcpy(void* dst, const void* src, size_t len);
void exit(int);
void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
}


