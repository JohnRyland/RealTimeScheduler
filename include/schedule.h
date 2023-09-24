/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include "timer.h"
#include "task.h"

#define UPDATE_SCHEDULE_RATE  1000   // every 1000 ticks schedule is updated
// ie this is the event horizon

// A scheduled_item is:
//  - an aperiodic task, or
//  - a single scheduled occurrence of a period task
typedef struct
{
  task_t*   task;
  tick_t    start_not_before;
  tick_t    complete_not_after;
  char      padding[7];
  bool      done;
} scheduled_item_t;


#define MAX_SCHEDULED_ITEMS    50

//extern unsigned items_in_scheduled_item_list;
//extern unsigned item_upto;
//extern unsigned index_sorted_upto;
//extern scheduled_item_t scheduled_item_list[MAX_SCHEDULED_ITEMS - 1];

unsigned get_items_in_scheduled_item_list();
unsigned& get_item_upto();
unsigned get_index_sorted_upto();
scheduled_item_t* get_scheduled_item_list();

#define items_in_scheduled_item_list  get_items_in_scheduled_item_list()
#define item_upto                     get_item_upto()
#define index_sorted_upto             get_index_sorted_upto()
#define scheduled_item_list           get_scheduled_item_list()

// return codes for various acceptance tests failures
enum acceptance_codes
{
  accepted = 0,
  bound_gt_period = 1,
  bound_gt_start_to_complete = 2,
  wait_for_not_present = 3,
  can_not_be_scheduled_with_the_other_tasks = 4,
  schedule_full = 5,
  scheduled_item_buffer_too_small = 6
};

void init_scheduler();

void run_scheduled_item(scheduled_item_t *item);

// compares items using the earliest deadline algorithm
int on_line_scheduled_item_cmp(const void *a, const void *b);

// order the schedule by ordering items in the scheduled item list
void sort_schedule();
bool add_to_scheduled_item_list(task_t *task, tick_t start_not_before, tick_t complete_not_after);
bool convert_periodic_tasks_to_scheduled_items_upto_event_horizon(task_t *item);
void online_scheduler();
void kill_task();

// bar representation of the scheduled tasks and how they will run
void draw_tasks();

// sets the real-time system going
void run_on_line_scheduler();

// save the current state of the scheduled list and its variables
void save_schedule_list_state();

// restore the state of the scheduled list and its variables
void restore_schedule_list_state();

// tries to work out if there is a viable schedule
acceptance_codes off_line_scheduler(task_t *task);

// returns true if it added task else it returns an error code
acceptance_codes request_to_add_task(void (*func_ptr)(), id_t task_name,
                                     id_t wait_for, tick_t start_not_before, ticks_t exec_bound,
                                     tick_t complete_not_after, ticks_t period,
                                     const char *name, unsigned x_pos, unsigned y_pos);


