/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <stdint.h>
#include "helpers.h"
#include "conio.h"
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
    for (int i = len-1; i >= 0; --i)
      dstc[i] = srcc[i];
  return dst;
}

void* memcpy(void* dst, const void* src, size_t len)
{
  return memmove(dst, src, len);
}

[[ noreturn ]]
void halt()
{
  for (;;)
    ;
}

[[ noreturn ]]
void exit(int code)
{
  gotoxy(0, 0);
  print_str("### exited with error code ");
  print_int(code);
  print_str(" ###");
  halt();
}

typedef int (*compare_t)(const void *, const void *);

static
void memswap(uint8_t* left, uint8_t* right, size_t width)
{
  for (int i = 0; i < width; ++i)
  {
    uint8_t tmp = left[i];
    left[i] = right[i];
    right[i] = tmp;
  }
}

static
void swap(uint8_t* base, int left, int right, int width)
{
  memswap(base + left*width, base + right*width, width);
}

static
int cmp(uint8_t* base, int left, int right, int width, compare_t comp)
{
  return comp(base + left*width, base + right*width);
}

static
void sift_down(uint8_t* base, int parent, int n, int w, compare_t comp)
{
  int child = 2*parent+1;
  if (child < n)
  {
    if (child+1 < n && cmp(base, child, child+1, w, comp) < 0)
      child++;
    if (cmp(base, parent, child, w, comp)<0)
    {
      swap(base, parent, child, w);
      sift_down(base, child, n, w, comp);
    }
  }
}

// https://stackoverflow.com/questions/23735879/what-is-wrong-with-this-heap-sort-implementation-in-c
static
void heapsort(uint8_t* base, int n, int w, compare_t comp)
{
  for (int i = (n/2)-1; i >= 0; i--)
    sift_down(base, i, n, w, comp);
  for (int active = n-1; active>0; active--)
  {
    swap(base, 0, active, w);
    sift_down(base, 0, active, w, comp);
  }
}

// https://stackoverflow.com/questions/19437135/internal-implementation-of-qsort
static
void qsort_internal(uint8_t *base, size_t width, size_t left, size_t right, compare_t comp)
{
  if (left >= right)
    return;
  swap(base, left, (left+right)/2, width);
  int last = left;
  for (int i = left+1; i <= right; ++i)
    if (cmp(base, i, left, width, comp) < 0)
      swap(base, last, i, width);
  swap(base, left, last, width);
  qsort_internal(base, width, left, last-1, comp);
  qsort_internal(base, width, last+1, right, comp);
}

static
void check_sorted(uint8_t* base, size_t nel, size_t width, compare_t compar)
{
  if (nel)
    for (int i = 1; i < nel; ++i)
      if (!cmp(base, i-1, i, width, compar))
      {
        gotoxy(0, 1);
        print_str(" ###### not sorted ###### ");
        print_int(i);
        exit(129);
      }
}

void qsort(void* base, size_t nel, size_t width, compare_t compar)
{
  qsort_internal((uint8_t*)base, width, 0, nel-1, compar);
  //heapsort((uint8_t*)base, nel, width, compar);
  check_sorted((uint8_t*)base, nel, width, compar);
}

}

