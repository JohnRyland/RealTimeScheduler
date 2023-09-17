/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "conio.h"
#include "timer.h"
#include "schedule.h"
#include "helpers.h"

#define EVENT_HORIZON          (2 * UPDATE_SCHEDULE_RATE)
//#define MAX_SCHEDULED_ITEMS    50

unsigned items_in_scheduled_item_list = 0;
unsigned item_upto = 0;
unsigned index_sorted_upto = 0;

scheduled_item_t scheduled_item_list[MAX_SCHEDULED_ITEMS - 1];

void run_scheduled_item(scheduled_item_t *item)
{
  run_task(item->task);
  if (item->task->last_exec_end > item->complete_not_after)
    item->task->deadline_failures++;
  item->done = true;
}

// compares items using the earliest deadline algorithm
static
int scheduled_item_cmp(const void *a, const void *b)
{
  const scheduled_item_t *a_item = static_cast<const scheduled_item_t*>(a);
  const scheduled_item_t *b_item = static_cast<const scheduled_item_t*>(b);
/* 
  // Probably shouldn't need to be sorting done items - that should be an assert
  if (a_item->done == true)
    return -1;
  if (b_item->done == true)
    return 1;
*/
  // having to wait_for another task has precedence over earliest deadline
  if (a_item->task->wait_for != 0)
    if (a_item->task->wait_for == b_item->task->task_name)
      return 1;

  if (b_item->task->wait_for != 0)
    if (b_item->task->wait_for == a_item->task->task_name)
      return -1;

  // sort items according to the "earliest deadline" algorithm
  return (a_item->complete_not_after < b_item->complete_not_after) ? -1 : 1;

  /*
  // "least slack algorithm"
  // this algorithm is more complicated and requires taking into
  // account all items that overlap, you can't just compare two
  // items and say that one goes before the other in the order
  // "earliest deadline" lends itself much better to using with qsort

  // simple cases where the 2 events don't overlap
  if (a_item->complete_not_after < b_item->start_not_before)
  return -1;
  if (a_item->start_not_before > b_item->complete_not_after)
  return 1;

  // else the 2 events overlap

  // comparing two events that start at the same time
  if (a_item->start_not_before == b_item->start_not_before)
  return 0;
  return (a_item->start_not_before < b_item->start_not_before) ? -1 : 1;
  */

}

// order the schedule by ordering items in the scheduled item list
void sort_schedule()
{
  qsort(static_cast<void *>(&scheduled_item_list[index_sorted_upto]), items_in_scheduled_item_list - index_sorted_upto, sizeof(scheduled_item_t), scheduled_item_cmp);
  index_sorted_upto = items_in_scheduled_item_list;
}

bool add_to_scheduled_item_list(task_t *task, tick_t start_not_before, tick_t complete_not_after)
{
  // due to using qsort as the sorting method, the list is a fixed size
  // array that will eventually run out
  // one possible way to avoid this is to divide the array into two halves
  // and copy the second half into the first half when the item we are up to
  // passes half way, then update all "up to" variables by subtracting the
  // size of half the array
  if (items_in_scheduled_item_list == MAX_SCHEDULED_ITEMS)
  {
    //critical_error("items in scheduled_item_list too large");
    return false;
  }

  scheduled_item_t *new_item = &scheduled_item_list[items_in_scheduled_item_list];
  new_item->done = false;
  new_item->task = task;
  new_item->start_not_before = start_not_before;
  new_item->complete_not_after = complete_not_after;

  if (items_in_scheduled_item_list && index_sorted_upto)
  {
    if (scheduled_item_list[index_sorted_upto - 1].complete_not_after > start_not_before)
    {
      // find first item (from the one we are up to) which completes before this one can start
      // this is the first item which doesn't overlap, we then need to re-sort from this first overlapping item to the last item
      // essentially, this will be doing an insertion, however it might not be as straight forward as doing a bsearch.
      unsigned old_index_sorted_upto = index_sorted_upto;
      index_sorted_upto = item_upto;
      while ((scheduled_item_list[index_sorted_upto].complete_not_after < start_not_before) && (index_sorted_upto <= old_index_sorted_upto))
      {
        index_sorted_upto++;
      }
      sort_schedule();
    }
  }

  items_in_scheduled_item_list++;
  return true;
}

bool convert_periodic_tasks_to_scheduled_items_upto_event_horizon(task_t *item)
{
  // if item has already been evaluated beyond this event horizon do nothing
  if (item->time_evaluated_upto > current_tick() + EVENT_HORIZON)
  {
    return true;
  }

  // evaluate from where it was evaluated upto last time it was converted
  tick_t schedule_time = item->time_evaluated_upto;

  // evaluate enough forward so the task will be evaluated upto at least
  // the next time the online scheduler will be run
  tick_t finish_time = item->time_evaluated_upto + UPDATE_SCHEDULE_RATE;

  // if the task is not to run after a given time, don't schedule it past that
  if (item->complete_not_after != 0)
  {
    if (item->complete_not_after < finish_time)
    {
      finish_time = item->complete_not_after;

      // if the task has run it's last execution, remove it from the
      // task list, or mark it with a boolean as being completed
      if (schedule_time > finish_time)
      {
        // remove_task_item(item);
        // needs to happen after the last time it's to be executed
        return true;
      }
    }
  }

  // convert the periodic task to a series of aperiodic events
  while (schedule_time < finish_time)
  {
    // its my understanding that a periodic task can be run anytime within
    // its period. I interpret the variables "start_not_before" and
    // "complete_not_after" as referring to the starting and completing of
    // the periodic events as a group.
    if (!add_to_scheduled_item_list(item, schedule_time, schedule_time + item->period))
    {
      return false;
    }
    schedule_time += item->period;
  }

  // update time_evaluated_upto variable
  item->time_evaluated_upto = schedule_time;
  return true;
}

static
void purge_completed_scheduled_items()
{
  // Only do this when the buffer is half full to avoid doing this every time
  if (items_in_scheduled_item_list >= MAX_SCHEDULED_ITEMS / 2)
  {
    /*
    // Validate the list
    for (i = 0; i < item_upto; ++i)
    {
      assert(scheduled_item_list[i].done == true);
    }
    */
    unsigned i = item_upto;
    if (i > 1)
    {
      items_in_scheduled_item_list -= i;
      if (item_upto < i)
      {
        print_str("item_upto ");
        print_int(item_upto);
        print_str(" but have done items up to ");
        print_int(i);
        exit(-1);
      }
      if (i != item_upto)
      {
        print_str("item_upto ");
        print_int(item_upto);
        print_str(" is not equal to ");
        print_int(i);
        exit(-1);
      }
      item_upto -= i;
      if (index_sorted_upto >= i)
        index_sorted_upto -= i;
      else
        index_sorted_upto = 0;
      memmove(static_cast<void*>(&scheduled_item_list[0]), static_cast<void*>(&scheduled_item_list[i]), sizeof(scheduled_item_t) * items_in_scheduled_item_list);
    }
  }
}

void online_scheduler()
{
  purge_completed_scheduled_items();
  for (unsigned i = 0; i < items_in_list; i++)
  {
    if (task_list[i].period != 0)
    {
      if (!convert_periodic_tasks_to_scheduled_items_upto_event_horizon(&task_list[i]))
      {
        // TODO: add error handling here
      }
    }
  }
  sort_schedule();

  //  refine_schedule();
  // an idea i have for improving the schedule
  // get item up to
  // scan backwards and forwards to items that no longer overlap it
  // order fully these items
  // get next item that doesn't overlap end of the last items
  // repeat until done for all item's in aperiodic list
}

static tick_t saved_current_tick;
static unsigned saved_items_in_schedule_list;
static unsigned saved_item_upto;
static unsigned saved_index_sorted_upto;
static scheduled_item_t saved_schedule_list[MAX_SCHEDULED_ITEMS - 1];

// save the current state of the scheduled list and its variables
void save_schedule_list_state()
{
  saved_current_tick = current_tick();
  saved_items_in_schedule_list = items_in_scheduled_item_list;
  saved_item_upto = item_upto;
  saved_index_sorted_upto = index_sorted_upto;
  for (unsigned item = 0; item < items_in_scheduled_item_list; item++)
    saved_schedule_list[item] = scheduled_item_list[item];
  //  saved_items_in_list = items_in_list;
  for (unsigned item = 0; item < items_in_list; item++)
    task_list[item].saved_time_evaluated_upto = task_list[item].time_evaluated_upto;
}

// restore the state of the scheduled list and its variables
void restore_schedule_list_state()
{
  set_current_tick(saved_current_tick);
  items_in_scheduled_item_list = saved_items_in_schedule_list;
  item_upto = saved_item_upto;
  index_sorted_upto = saved_index_sorted_upto;
  for (unsigned item = 0; item < items_in_scheduled_item_list; item++)
    scheduled_item_list[item] = saved_schedule_list[item];
  // need to reset the tick periodic tasks have been evaluated upto

  // task was only tentertively added to task_list
  // removing the task this way doesn't properly deallocate memory
  //  items_in_list = saved_items_in_list;
  items_in_list--;
  for (unsigned item = 0; item < items_in_list; item++)
    task_list[item].time_evaluated_upto = task_list[item].saved_time_evaluated_upto;
}

// trys to work out if there is a viable schedule
acceptance_codes off_line_scheduler(task_t *task)
{
  // most of the code of this function seems to be unstable and is
  // why it is commented
  // i can't work out what is wrong with it, although it is a little
  // dodgy the way it goes about some of the things
  // the basic idea is that it simulates the time elasped in executing
  // the schedule that the on line scheduler will produce
  // it will simulate it until it finds that a deadline will be missed
  // with the new schedule, or it will simulate for 5 event horizons
  // past when it encounters the first instance of the task
  // this by no means proves the schedule is 100% viable
  /*
  int test_ahead_x_event_horizons = 5;
  bool done = false;
  bool found_task = false;

  save_schedule_list_state();

  if (task->period == NULL)
  {
    if ((task->start_not_before != NULL) && (task->complete_not_after != NULL))
    {
      if (!add_to_scheduled_item_list(task, task->start_not_before, task->complete_not_after))
      {
        return scheduled_item_buffer_too_small;
      }
    }
  }
  else
  {
    if (!convert_periodic_tasks_to_scheduled_items_upto_event_horizon(task))
    {
      return scheduled_item_buffer_too_small;
    }
  }

  while (!done)
  {
    if (found_task == true)
    {
      test_ahead_x_event_horizons--;
    }

    if (test_ahead_x_event_horizons == 0)
    {
      done = true;
    }

    // i think it's better to test the viability of what will happen
    // rather than an ideal possibility of how things could be scheduled
    online_scheduler();

    tick anticipated_completion_of_last_task = 0;

    // for all scheduled items from the current item we are upto
    for (int item = item_upto; item < items_in_scheduled_item_list; item++)
    {
      tick anticipated_start_tick, bar_start, bar_end;

      if (scheduled_item_list[item]->task->task_name == task->task_name)
      {
        found_task = true;
      }

      // work out where we expect the item will begin
      anticipated_start_tick = scheduled_item_list[item]->start_not_before;
      if (anticipated_start_tick < anticipated_completion_of_last_task)
      {
        anticipated_start_tick = anticipated_completion_of_last_task;
      }
      anticipated_completion_of_last_task = anticipated_start_tick
                                              + scheduled_item_list[item]->task->exec_bound;
      if (anticipated_completion_of_last_task > scheduled_item_list[item]->complete_not_after)
      {
        // put things back the way they were before
        restore_schedule_list_state();
        return can_not_be_scheduled_with_the_other_tasks;
      }
    }

    item_upto = items_in_scheduled_item_list - 1;
    current_tick = scheduled_item_list[items_in_scheduled_item_list - 1]->complete_not_after;
  }

  restore_schedule_list_state();
  */

  if (task->period == 0)
  {
    if ((task->start_not_before != 0) && (task->complete_not_after != 0))
    {
      if (!add_to_scheduled_item_list(task, task->start_not_before, task->complete_not_after))
      {
        return scheduled_item_buffer_too_small;
      }
    }
  }
  else
  {
    if (!convert_periodic_tasks_to_scheduled_items_upto_event_horizon(task))
    {
      return scheduled_item_buffer_too_small;
    }
  }

  return accepted;
}

// returns true if it aded task else it returns an error code
acceptance_codes request_to_add_task(void (*func_ptr)(), id_t task_name,
                                     id_t wait_for, tick_t start_not_before, ticks_t exec_bound,
                                     tick_t complete_not_after, ticks_t period,
                                     const char *name, unsigned x_pos, unsigned y_pos)
{

  // reject tasks that obviously will fail and then use the
  // offline_scheduler to determine if there is a viable schedule

  // in the case of two periodic tasks, I'm not sure about the variable
  // "wait_for". It could mean that each occurrence has to wait for each
  // occurrence of the other task. This can be guaranteed if the task
  // waiting for the first task, starts not before the first task starts
  // plus it's period. The periods need to be equal or made equal, and
  // the "complete_not_after" variable needs to be made the same plus
  // the period aswell.

  if (wait_for != 0)
  {
    if (search_for_task_in_schedule(task_name) == nullptr)
    {
      return wait_for_not_present;
    }
  }

  if ((start_not_before != 0) && (complete_not_after != 0))
  {
    if (start_not_before + exec_bound > complete_not_after)
    {
      return bound_gt_start_to_complete;
    }
  }

  if (period != 0)
  {
    if (exec_bound > period)
    {
      return bound_gt_period;
    }
  }

  if (add_task_to_schedule(func_ptr, task_name, wait_for, start_not_before,
        exec_bound, complete_not_after, period, name, x_pos, y_pos) == false)
  {
    return schedule_full;
  }

  return off_line_scheduler(&task_list[items_in_list - 1]);
}


