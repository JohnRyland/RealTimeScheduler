/*
  RTOS Runtime
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "module_manager.h"
#include "random.h"
//#include "conio.h"
#include "utilities.h"
#include "kernel/exception_handler.h"

unsigned k_random(unsigned upper_bound)
{
  if (!modules_initialized())
    k_critical_error(10, "Function unavailable until modules initialized\n");
  module_t const* random_module = find_module_by_class(module_class::RANDOM_DEVICE);
  return ((random_device_vtable_t*)random_module->vtable)->random(0, upper_bound);
}

typedef int (*compare_t)(const void *, const void *);

static
void memswap(uint8_t* left, uint8_t* right, size_t width)
{
  for (size_t i = 0; i < width; ++i)
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
  for (size_t i = left+1; i <= right; ++i)
    if (cmp(base, i, left, width, comp) < 0)
      swap(base, last, i, width);
  swap(base, left, last, width);
  qsort_internal(base, width, left, last-1, comp);
  qsort_internal(base, width, last+1, right, comp);
}

static
const void* bsearch_internal(const void* key, const void* base, int left, int right, size_t size, compare_t comp)
{
   if (left >= right)
      return nullptr;
   int mid = left + (right - left) / 2;
   const uint8_t* mid_ptr = (const uint8_t*)base;
   mid_ptr += mid * size;
   int res = comp(key, mid_ptr);
   if (res < 0)
     return bsearch_internal(key, base, left, mid, size, comp);
   if (res > 0)
     return bsearch_internal(key, base, mid+1, right, size, comp);
   return (const void*)mid_ptr;
}

static
void check_sorted(uint8_t* base, size_t nel, size_t width, compare_t comp)
{
  if (nel)
    for (size_t i = 1; i < nel; ++i)
      if (!cmp(base, i-1, i, width, comp))
        k_critical_error(129, " ###### not sorted ###### ");
}

void k_qsort(void* base, size_t nel, size_t width, compare_t compare_func)
{
  static const bool use_heapsort = true;
  if (nel > 5000 || width > 100)   // Make sure we aren't using this on anything big
    k_critical_error(129, " ###### bad sort input ###### ");
  if (use_heapsort)
    heapsort((uint8_t*)base, nel, width, compare_func);
  else
    qsort_internal((uint8_t*)base, width, 0, nel-1, compare_func);
  check_sorted((uint8_t*)base, nel, width, compare_func);
}

const void *k_bsearch(const void *key, const void *base, size_t nmemb, size_t size, compare_t compare_func)
{
  return bsearch_internal(key, base, 0, nmemb, size, compare_func);
}
