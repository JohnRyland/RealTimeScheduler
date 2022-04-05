/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "helpers.h"
#include "conio.h"

static unsigned status_row = 5;

void status_message(const char *message)
{
  gotoxy(2,status_row++);
  puts2(message);
}

[[ noreturn ]]
void critical_error(const char *error_message)
{
  clrscr();
  printf("critical error: %s\n", error_message);
  timer.disable();
  exit(255);
}

void status_to_adding_a_task(acceptance_codes status, const char *message)
{
  char buffer[80];
  const char *error_msgs[6] = {
    "task accepted",
    "exec_bound > period",
    "start + exec_bound > deadline",
    "must topologically sort requests in advance",
    "can't be scheduled with the other tasks",
    "schedule full"
  };

  if (status == accepted)
  {
    sprintf(buffer, "sucessfully added: %s", message);
    status_message(buffer);
  }
  else
  {
    sprintf(buffer,"couldn't add: %s",message);
    status_message(buffer);
    sprintf(buffer,"  reason: %s",error_msgs[status]);
    status_message(buffer);
  }
}

static void kill_task(void* user_data)
{
  task_t* task = reinterpret_cast<task_t*>(user_data);

  // Allow up to 3 deadline failures, then quit if keeps happening
  if (task->deadline_failures < 3)
  {
    ++task->deadline_failures;
    return;
  }

  // my pre-emptor is unstable and causes the system to hang so
  // this function isn't called any more
  timer.uninstall_preemptor(); //  this should happen automatically 
  
  //critical_error("task has executed longer than it's exec_bound");
  clrscr();
  gotoxy(0,0);
  printf("critical error: task has executed longer than it's exec_bound\n");
  timer.disable();
  printf("Current tick:\n");
  printf("   Tick:                      %i\n", current_tick());
  printf("Task details:\n");
  printf("   Name:                      %s\n", task->name);
  printf("   wait_for:                  %i\n", task->wait_for);
  printf("   start_not_before:          %i\n", task->start_not_before);
  printf("   exec_bound:                %i\n", task->exec_bound);
  printf("   complete_not_after:        %i\n", task->complete_not_after);
  printf("   period:                    %i\n", task->period);
  printf("   time_evaluated_upto:       %i\n", task->time_evaluated_upto);
  printf("   saved_time_evaluated_upto: %i\n", task->saved_time_evaluated_upto);
  printf("   last_exec_start:           %i\n", task->last_exec_start);
  printf("   last_exec_end:             %i\n", task->last_exec_end);
  printf("   last_exec_time:            %i\n", task->last_exec_time);
  printf("   min_exec_time:             %i\n", task->min_exec_time);
  printf("   max_exec_time:             %i\n", task->max_exec_time);
  printf("   total_exec_time:           %i\n", task->total_exec_time);
  printf("   average_exec_time:         %i\n", task->average_exec_time);
  printf("   times_called:              %i\n", task->times_called);
  printf("   deadline_failures:         %i\n", task->deadline_failures);
  exit(255);
}

// bar representation of the scheduled tasks and how they will run
void draw_tasks()
{
  // clear bars in display area
  for (unsigned i = 0; i < 10; i += 2)
  {
    gotoxy(19,40 + i);
    puts2(" \xb3                                                          ");
  }

  tick_t anticipated_completion_of_last_task = 0;

  // display all scheduled items from the current item we are upto
  for (unsigned item = item_upto; item < items_in_scheduled_item_list; item++)
  {
    // display in row corresponding to tasks id
    unsigned display_row = 2 * scheduled_item_list[item].task->task_name;
    if (display_row < 10)
    {
      tick_t anticipated_start_tick, bar_start, bar_end;

      // work out where we expect the item will begin
      anticipated_start_tick = scheduled_item_list[item].start_not_before;
      if (anticipated_start_tick < anticipated_completion_of_last_task)
      {
        anticipated_start_tick = anticipated_completion_of_last_task;
      }
      anticipated_completion_of_last_task = anticipated_start_tick 
                                             + scheduled_item_list[item].task->exec_bound;

      bar_start = ((anticipated_start_tick - current_tick()) / 20) + 20;
      bar_end = bar_start + scheduled_item_list[item].task->exec_bound / 20;

      gotoxy(1, 40 + display_row);
      puts2(scheduled_item_list[item].task->name);

      // draw it as a bar
      for (unsigned int i = bar_start; i < bar_end; i++)
      {
        if ((i > 18) && (i < 79))
        {
          gotoxy(i, 40 + display_row);
          putch('\xdb');
        }
      }

      // draw characters to show the start and complete by constraints
      tick_t start_not_before = ((scheduled_item_list[item].start_not_before - current_tick()) / 20) + 19;
      tick_t complete_not_after = ((scheduled_item_list[item].complete_not_after - current_tick()) / 20) + 18;

      if ((start_not_before > 18) && (start_not_before < 79))
      {
        gotoxy(start_not_before, 40 + display_row);
        putch('\xb3');
      }
      if ((complete_not_after > 18) && (complete_not_after < 79))
      {
        gotoxy(complete_not_after,40 + display_row);
        putch('\xba');
      }
    }
  }
}

// sets the realtime system going
void run_on_line_scheduler()
{
  // set timer going
  timer.enable();

  for (item_upto = 0; item_upto < items_in_scheduled_item_list; item_upto++)
  {
    scheduled_item_t* item = &scheduled_item_list[item_upto];

    // wait till its time to run the next scheduled item
    while (current_tick() < item->start_not_before)
    {
      gotoxy(2,4);
      printf("current tick: %u ",current_tick());
      // wait for the next tick
      // this will call draw_tasks
      delay(1, item->start_not_before);

      if (kbhit())
      {
        int ch = getch();
        switch(ch)
        {
          case  27: // ESC
          case 'q':
          case 'x': critical_error("user abort"); // break;
          case ' ': timer.suspend(); getch(); timer.resume(); break;
          case '+':
          case '=': timer.speed_up(); break;
          case '_':
          case '-': timer.slow_down(); break;
          default:  break;
        }
      }
    }

    // pre-empt task about to be run if it goes over its exec_bound
    while (timer.uninstall_preemptor() != true)
    {
      /* try again */
    }

    while (timer.install_preemptor(current_tick() + item->task->exec_bound, kill_task, item->task) != true)
    {
      /* try again */
    }

    // run it
    run_scheduled_item(item);

    while (timer.uninstall_preemptor() != true)
    {
      /* try again */
    }
  }

  timer.disable();
}


void test_deterministic()
{
  delay(49);
}

void test_exponential()
{
  delay(random(50));
}

void test_binary()
{
  random(2) ? delay(49) : delay(random(50));
}

void test_added_on_the_fly()
{
  delay(99);
}

void test_adding_task_on_the_fly()
{
  status_to_adding_a_task(request_to_add_task(test_added_on_the_fly, 5, 0, 20000, 100, 30000, 1000, "Task added on the fly", 28, 26), "task added on the fly");
}

