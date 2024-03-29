/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#include "helpers.h"
#include "conio.h"
#include "debug_logger.h"
#include "exception_handler.h"

static
unsigned status_row = 5;

void initialize_status()
{
  status_row = 5;
}

void print_str(const char* str)
{
  k_log_fmt(DEBUG, "%s", str);
}

/*
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
  //print_str("0");
  //return;
  print_number<10>(val);
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

*/

void status_message(const char *message)
{
  gotoxy(2,status_row++);
  print_str(message);
}

void status_to_adding_a_task(acceptance_codes status, const char *message)
{
  static const char *error_msgs[6] = {
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

//static
void print_str_int(const char* str, int val)
{
  k_log_fmt(DEBUG, "%s%i \n", str, val);
//  print_str(str);
//  print_int(val);
//  print_str("\n");
}

static
void kill_task(void* user_data)
{
  task_t* task = reinterpret_cast<task_t*>(user_data);

  // Allow up to 3 deadline failures, then quit if keeps happening
  //if (task->deadline_failures < 3)
  //{
  //  ++task->deadline_failures;
  //  return;
  //}

  // my pre-emptor is unstable and causes the system to hang so
  // this function isn't called any more
  //timer.uninstall_preemptor(); //  this should happen automatically 
  //exit(133);
  
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
  gotoxy(2,4);
  print_str_int("current tick: ", current_tick());

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
      k_log_fmt(NORMAL, scheduled_item_list[item].task->name);

      // draw it as a bar
      for (unsigned int i = bar_start; i < bar_end; i++)
      {
        if ((i > 20) && (i < 79))
        {
          gotoxy(i, 40 + display_row);
          k_log_fmt(DEBUG, "\xdb");
        }
      }

      // draw characters to show the start and complete by constraints
      tick_t start_not_before = ((scheduled_item_list[item].start_not_before - current_tick()) / 20) + 19;
      tick_t complete_not_after = ((scheduled_item_list[item].complete_not_after - current_tick()) / 20) + 18;

      if ((start_not_before > 19) && (start_not_before < 79))
      {
        gotoxy(start_not_before, 40 + display_row);
        k_log_fmt(DEBUG, "\xb3");
        //putch('\xb3');
      }
      if ((complete_not_after > 19) && (complete_not_after < 79))
      {
        gotoxy(complete_not_after,40 + display_row);
        k_log_fmt(DEBUG, "\xba");
        //putch('\xba');
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
//      delay(1, item->start_not_before);

      tick_t finish_at = current_tick() + 1;
      while (current_tick() < finish_at)
      {
        // wait for next tick
        
        // wait for events
        
        // TODO: this is where can run non-realtime processes in this free time
        // they will get pre-empted when the timer goes off
        asm volatile ( "hlt" );

      }


      if (kbhit())
      {
        int ch = getch();
        switch(ch)
        {
          case  27: exit(0); break; // ESC
          case 'x': k_panic(); break;
          case 'q': k_critical_error(0, "user abort"); break;

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

    const int fudgeMargin = 20;  // TODO: Annoyingly this is here to make things work, but goal should be to reduce this to 0
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
  delay(5);
}

void test_exponential()
{
  delay(k_random(5));

  // Deliberately cause a fault to test the fault exception handling

  // Generate a division by zero fault
  //asm volatile ("xorl %%eax,%%eax\nxorl %%edx,%%edx\nidivl %%eax" : : : "eax" );

  // Generate a seg fault / general protection fault
  //asm volatile ("movl $0xF0,%%eax\nmovl %%eax,%%ds\nxorl %%eax,%%eax\ndecl %%eax\nmovl %%eax,(%%eax)" : : : "eax" );

  // Generate a break point
  //asm volatile ("int $0x3");

  // Generate an invalid opcode
  //asm volatile (".byte 0xF0, 0xF0");
}

void test_binary()
{
  k_random(2) ? delay(1) : delay(k_random(10));
}

void test_added_on_the_fly()
{
  delay(9);
}

void test_adding_task_on_the_fly()
{
  status_to_adding_a_task(request_to_add_task(test_added_on_the_fly, 6, 0, 20000, 10, 30000, 1000, "Task added on the fly", 54, 26), "task added on the fly");
}
