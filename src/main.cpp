/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "conio.h"
#include "schedule.h"
#include "helpers.h"

static
void repeat_putch(char ch, int repeats)
{
  for (int i = 0; i < repeats; ++i)
    putch(ch);
}

static
void draw_frame_row(unsigned x, unsigned y, char ch1, char ch2, char ch3, char ch4)
{
  gotoxy(x, y);
  putch(ch1);
  repeat_putch('\xc4', 25);
  putch(ch2);
  repeat_putch('\xc4', 25);
  putch(ch3);
  repeat_putch('\xc4', 25);
  putch(ch4);
}

static
void draw_bar_row(unsigned x, unsigned y, char ch1)
{
  gotoxy(x, y);
  repeat_putch('\xc4', 19);
  putch(ch1);
  repeat_putch('\xc4', 59);
}

int main()
{
  // draw a title screen with instructions
  clrscr();
  puts2("                                     toy-rtos");
  puts2("                                REALTIME SCHEDULER");
  puts2("                                  by John Ryland");
  puts2("\n\nkeyboard functions:\n");
  puts2("  'Q' 'X' or 'ESC' all exit the program immediately");
  puts2("  ' ' SPACE BAR pauses everything until a key is pressed");
  puts2("  '+' '=' or UP ARROW speeds things up");
  puts2("  '-' or DOWN ARROW slows things up");
  puts2("\n\nmeaning of characters in display:\n");
  puts2("  '\xb3' start_not_before");
  puts2("  '\xba' complete_not_after");
  puts2("  '\xdb\xdb\xdb\xdb\xdb' where it is anticipated it will run");
  puts2("\npress any key to continue");
  getch();

  // change screen mode
  textmode(C4350);
  clrscr();

  // draw borders and windows with line drawing characters
  for (unsigned i = 1; i < 37; i++)
  {
    gotoxy(1, i); putch('\xb3');
    if (i > 13)
    {
      gotoxy(27, i); putch('\xb3');
    }
    gotoxy(53, i); putch('\xb3');
    gotoxy(79, i); putch('\xb3');
  }
  
  draw_frame_row(1,  1, '\xda', '\xc4', '\xc2', '\xbf');
  gotoxy(2, 2);
  puts2("Status Window");
  draw_frame_row(1,  3, '\xc3', '\xc4', '\xc5', '\xb4');
  draw_frame_row(1, 13, '\xc3', '\xc2', '\xc5', '\xb4');
  draw_frame_row(1, 15, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 25, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 27, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 37, '\xc0', '\xc1', '\xc1', '\xd9');
  
  gotoxy(23, 38);
  puts2("Bar representation of tasks window");
  draw_bar_row(1, 39, '\xc2');
  for (unsigned i = 0; i < 8; i += 2)
  {
    gotoxy(1, 40 + i); repeat_putch(' ', 19); putch('\xb3');
    draw_bar_row(1, 41 + i, '\xc5');
  }
  gotoxy(1, 48); repeat_putch(' ', 19); putch('\xb3');
  draw_bar_row(1, 49, '\xc1');

  // this task must be here as it needs to be the first one to run
  status_to_adding_a_task(request_to_add_task(online_scheduler,            0, 0,     0,  20,     0, UPDATE_SCHEDULE_RATE - 1,  "Online Scheduler", 54, 2), "online scheduler");
  // add user tasks to be scheduled
  status_to_adding_a_task(request_to_add_task(test_deterministic,         10, 0,     0, 100,     0,                       50,    "unaccept test1",  2, 14), "task with exec_bound > period");
  status_to_adding_a_task(request_to_add_task(test_deterministic,          1, 0,     0,  50,     0,                      200,     "Deterministic",  2, 14), "deterministic");
  status_to_adding_a_task(request_to_add_task(test_exponential,            2, 0,     0,  50,     0,                      700,       "Exponential", 28, 14), "exponential");
  status_to_adding_a_task(request_to_add_task(test_binary,                 3, 0,     0,  50,     0,                      500,            "Binary", 54, 14), "binary");
  status_to_adding_a_task(request_to_add_task(test_adding_task_on_the_fly, 4, 0, 10000,  50, 10500,                        0, "Exec another task",  2, 26), "on the fly task");

  init_timer_driver();

  // set the realtime system going
  run_on_line_scheduler();
  return 0;
}

