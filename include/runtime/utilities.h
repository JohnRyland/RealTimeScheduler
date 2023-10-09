/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

typedef int (*compare_t)(const void *, const void *);

void k_qsort(void *base, size_t nel, size_t width, compare_t compare_func);
const void *k_bsearch(const void *key, const void *base, size_t nmemb, size_t size, compare_t compare_func);
unsigned k_random(unsigned x);
