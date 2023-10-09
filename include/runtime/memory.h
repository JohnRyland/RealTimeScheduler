/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

void* mem_set(void* dst, int val, size_t len);
void* mem_move(void* dst, const void* src, size_t len);
void* mem_cpy(void* dst, const void* src, size_t len);
int mem_cmp(const void* dst, const void* src, size_t len);
void mem_swap(void* first, void* second, size_t len);
