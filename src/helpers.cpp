/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "helpers.h"
#include "conio.h"

static
unsigned status_row = 5;

void init_status()
{
  status_row = 5;
}

void print_str(const char* str)
{
  while (*str)
    putch(*str++);
}

template <unsigned BASE>
static inline
void print_number(unsigned val)
{
  static const uint8_t base = (BASE > 16) ? 16 : BASE;  // clamp it to 16 or less
  static const char digits[] = "0123456789ABCDEF";      // character map
  // char buf[22] = {}; // with intializiation like this, it breaks at runtime if build with x86_64-elf-gcc.
  char buf[22];                     // big enough for 64-bit number in decimal
  buf[21] = 0;                      // put nul at what will be the end of the string
  char* ptr = buf + 21;             // point to the destination in the string of the last character of the number
  do {                              // loop at least once, so if val is zero it will put at least one character which is '0'.
    *(--ptr) = digits[val % base];  // keep writing out the next number working to the left (to the most significant digits)
  } while (val /= base);            // until we have processed every decimal digit.
  print_str(ptr);                   // now print the resulting string from the most significant digit we last wrote.
}

void print_int(int val)
{
  print_number<10>(val);
}

void status_message(const char *message)
{
  gotoxy(2,status_row++);
  print_str(message);
}

[[ noreturn ]]
void critical_error(const char *error_message)
{
  clrscr();
  print_str("critical error: ");
  print_str(error_message);
  timer.disable();
  exit(132);
}

void status_to_adding_a_task(acceptance_codes status, const char *message)
{
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
    status_message("sucessfully added: ");
    print_str(message);
  }
  else
  {
    status_message("couldn't add: ");
    print_str(message);
    status_message("  reason: ");
    print_str(error_msgs[status]);
  }
}

static
void print_str_int(const char* str, int val)
{
  print_str(str);
  print_int(val);
  print_str("\n");
}

static
void kill_task(void* user_data)
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
  exit(133);
  
  //critical_error("task has executed longer than it's exec_bound");
  clrscr();
  gotoxy(0,0);
  print_str    ("critical error: task has executed longer than it's exec_bound\n");
  timer.disable();
  print_str    ("Current tick:\n");
  print_str_int("   Tick:                      ", current_tick());
  print_str    ("Task details:\n");
  print_str    ("   Name:                      "); print_str(task->name); print_str("\n");
  print_str_int("   wait_for:                  ", task->wait_for);
  print_str_int("   start_not_before:          ", task->start_not_before);
  print_str_int("   exec_bound:                ", task->exec_bound);
  print_str_int("   complete_not_after:        ", task->complete_not_after);
  print_str_int("   period:                    ", task->period);
  print_str_int("   time_evaluated_upto:       ", task->time_evaluated_upto);
  print_str_int("   saved_time_evaluated_upto: ", task->saved_time_evaluated_upto);
  print_str_int("   last_exec_start:           ", task->last_exec_start);
  print_str_int("   last_exec_end:             ", task->last_exec_end);
  print_str_int("   last_exec_time:            ", task->last_exec_time);
  print_str_int("   min_exec_time:             ", task->min_exec_time);
  print_str_int("   max_exec_time:             ", task->max_exec_time);
  print_str_int("   total_exec_time:           ", task->total_exec_time);
  print_str_int("   average_exec_time:         ", task->average_exec_time);
  print_str_int("   times_called:              ", task->times_called);
  print_str_int("   deadline_failures:         ", task->deadline_failures);
  exit(134);
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
      print_str_int("current tick: ", current_tick());
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

    const int fudgeMargin = 5;
    while (timer.install_preemptor(current_tick() + item->task->exec_bound + fudgeMargin, kill_task, item->task) != true)
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

  // Delibrally cause a divide by zero to test the fault/expection handling
  int x = 0;
  int y = 10 / x;
  x = y + 10;
  print_int(x);
}

void test_adding_task_on_the_fly()
{
  status_to_adding_a_task(request_to_add_task(test_added_on_the_fly, 5, 0, 20000, 100, 30000, 1000, "Task added on the fly", 28, 26), "task added on the fly");
}

