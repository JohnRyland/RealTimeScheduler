/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include "schedule.h"

typedef int size_t;

extern "C"
{

// runtime functions used that needs to be provided
[[ noreturn ]]
void exit(int);
void* memset(void* dst, int val, size_t len);
void* memmove(void* dst, const void* src, size_t len);
void* memcpy(void* dst, const void* src, size_t len);
void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));

}

// output helpers (avoids using printf)
void print_str(const char* str);
void print_int(int val);

[[ noreturn ]]
void critical_error(const char *error_message);
void status_message(const char *message);

void status_to_adding_a_task(acceptance_codes status, const char *message);
void test_deterministic();
void test_exponential();
void test_binary();
void test_added_on_the_fly();
void test_adding_task_on_the_fly();

