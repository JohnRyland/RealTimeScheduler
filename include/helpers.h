/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include "schedule.h"
#include "runtime.h"


void init_status();
void status_to_adding_a_task(acceptance_codes status, const char *message);
void status_message(const char *message);

void test_deterministic();
void test_exponential();
void test_binary();
void test_added_on_the_fly();
void test_adding_task_on_the_fly();
