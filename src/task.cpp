/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "conio.h"
#include "timer.h"
#include "task.h"

// Because i'm lazy I thought it easiest to use a constant length array
// for my schedule_list instead of using a linked list or other data struct
unsigned items_in_list = 0;
task_t task_list[MAX_TASKS - 1];

bool add_task_to_schedule(void (*func_ptr)(), id_t task_name, id_t wait_for,
			     tick_t start_not_before, ticks_t exec_bound,
			     tick_t complete_not_after, ticks_t period,
			     const char *name, unsigned x_pos, unsigned y_pos)
{
  if (items_in_list == MAX_TASKS)
    return false;

  task_t* new_item = &task_list[items_in_list];
  new_item->func_ptr = func_ptr;
  new_item->task_name = task_name;
  new_item->wait_for = wait_for;
  new_item->start_not_before = start_not_before;
  new_item->exec_bound = exec_bound;
  new_item->complete_not_after = complete_not_after;
  new_item->period = period;

  if (start_not_before == 0)
    new_item->time_evaluated_upto = current_tick();
    // should be zero except for tasks added whilst running
  else
    new_item->time_evaluated_upto = start_not_before;

  new_item->last_exec_start = 0;
  new_item->last_exec_end = 0;
  new_item->last_exec_time = 0;
  new_item->min_exec_time = UINT_MAX;//-1;
  new_item->max_exec_time = 0;
  new_item->total_exec_time = 0;
  new_item->average_exec_time = 0;
  new_item->times_called = 0;
  new_item->deadline_failures = 0;

  new_item->name = name;
  new_item->x_pos = x_pos;
  new_item->y_pos = y_pos;

  items_in_list++;
  return true;
}

task_t *search_for_task_in_schedule(id_t task_name)
{
  for (unsigned i = 0; i < items_in_list; i++)
    if (task_list[i].task_name == task_name)
      return &task_list[i];
  return nullptr;
}

static
void calculate_stats(task_t *item)
{
  item->last_exec_time = item->last_exec_end - item->last_exec_start;
  item->total_exec_time += item->last_exec_time;
  item->average_exec_time = item->total_exec_time / item->times_called;

  if (item->last_exec_time > item->max_exec_time)
    item->max_exec_time = item->last_exec_time;

  if (item->last_exec_time < item->min_exec_time)
    item->min_exec_time = item->last_exec_time;
}

static
void display_item(task_t *item)
{
  unsigned x = item->x_pos, y = item->y_pos;
  gotoxy(x,y++);  printf("%s", item->name);
  gotoxy(x,y++);
  gotoxy(x,y++);  printf("times called:      %u ",item->times_called);
  gotoxy(x,y++);  printf("deadline failures: %u ",item->deadline_failures);
  gotoxy(x,y++);  printf("last exec time:    %u ",item->last_exec_time);
  gotoxy(x,y++);  printf("min exec time:     %u ",item->min_exec_time);
  gotoxy(x,y++);  printf("max exec time:     %u ",item->max_exec_time);
  gotoxy(x,y++);  printf("total exec time:   %u ",item->total_exec_time);
  gotoxy(x,y++);  printf("average exec time: %u ",item->average_exec_time);
  gotoxy(x,y++);  printf("period:            %u ",item->period);
  gotoxy(x,y++);  printf("evaluated upto:    %u ",item->time_evaluated_upto);
}

void run_task(task_t *item)
{
  item->last_exec_start = current_tick();
  item->func_ptr();
  item->last_exec_end = current_tick();
  item->times_called++;
  calculate_stats(item);
  display_item(item);
}

