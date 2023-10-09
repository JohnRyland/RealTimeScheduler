/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"
#include "timer.h"

//typedef uint32_t id_t;

// return codes for various acceptance tests failures
enum class task_acceptance_t : uint8_t
{
  accepted = 0,
  bound_gt_period = 1,
  bound_gt_start_to_complete = 2,
  wait_for_not_present = 3,
  can_not_be_scheduled_with_the_other_tasks = 4,
  schedule_full = 5,
  scheduled_item_buffer_too_small = 6
};

/*
void run_scheduled_item(scheduled_item_t *item);
// order the schedule by ordering items in the scheduled item list
bool add_to_scheduled_item_list(task_t *task, tick_t start_not_before, tick_t complete_not_after);
void kill_task();
// sets the real-time system going
void run_on_line_scheduler();
// tries to work out if there is a viable schedule
acceptance_codes off_line_scheduler(task_t *task);
// returns true if it added task else it returns an error code
*/

/*
struct task_t
{
  void (*func_ptr)();
  id_t task_name;
  id_t wait_for;
  tick_t start_not_before;
  ticks_t exec_bound;
  tick_t complete_not_after;
  ticks_t period;
  //const char *name;
  //unsigned x_pos;
  //unsigned y_pos;
};
*/

struct scheduler_vtable_t
{
  bool (*initialize)();
  task_acceptance_t (*add_task)(task_t* task);
};
