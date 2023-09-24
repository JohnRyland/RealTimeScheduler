/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include <stdint.h>

// Types
typedef uint32_t ticks_t;
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


#define MAX_TASKS             100
//extern unsigned items_in_list;
//extern task_t task_list[MAX_TASKS - 1];


unsigned get_items_in_list();
task_t* get_task_list();
#define items_in_list   get_items_in_list()
#define task_list       get_task_list()

void init_tasks();

bool add_task_to_schedule(task_entry_t func_ptr,
                          id_t         task_name,
                          id_t         wait_for,
                          tick_t       start_not_before,
                          ticks_t      exec_bound,
                          tick_t       complete_not_after,
                          ticks_t      period,
                          const char*  name,
                          unsigned     x_pos,
                          unsigned     y_pos);

task_t *search_for_task_in_schedule(id_t task_name);

void run_task(task_t *item);

