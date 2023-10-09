/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "time.h"

enum class schedule_type : uint8_t
{
  LOW_PRIORITY,
  NORMAL_PRIORITY,
  HIGH_PRIORITY,
  REALTIME,
};

// Types
typedef uint32_t count_t;
typedef uint32_t id_t;
typedef void (*task_entry_t)();

struct task_t
{
  // Scheduler required parameters
  task_entry_t  func_ptr;
  id_t          task_name;
  id_t          wait_for;
  tick_t        start_not_before;
  ticks_t       exec_bound;
  tick_t        complete_not_after;
  tick_t        period;
  tick_t        time_evaluated_upto;
  tick_t        saved_time_evaluated_upto;

  // Statistical analysis parameters
  tick_t        last_exec_start;
  tick_t        last_exec_end;
  ticks_t       last_exec_time;
  ticks_t       min_exec_time;
  ticks_t       max_exec_time;
  ticks_t       total_exec_time;
  ticks_t       average_exec_time;
  count_t       times_called;
  count_t       deadline_failures;
  int           padding;

  // Display parameters (where output is printed)
  const char*   name;
  unsigned      x_pos;
  unsigned      y_pos;
};
