/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../../include/types/tasks.h"

//  task_t* (*allocate_task)();

#define MAX_TASKS             100
//extern unsigned items_in_list;
//extern task_t task_list[MAX_TASKS - 1];


unsigned get_items_in_list();
task_t* get_task_list();
#define items_in_list   get_items_in_list()
#define task_list       get_task_list()

void initialize_tasks();

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


