/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>

#include "conio.h"
#include "timer.h"


// Global variables
static volatile tick_t current_tick_ = 0;   // number of ticks since timer was enabled
static unsigned timer_speed = 1000;
static std::atomic_bool timer_active(false);
static tick_t preempt_at_tick = 0;
static preemptor_t user_preemptor = nullptr;
static void* user_preemptor_data = nullptr;
static std::atomic_bool preemptor_active(false);
static std::atomic_bool installed_timer_interrupt_in_service(false);
static dispatch_queue_t queue;
static dispatch_source_t timer1;


// Externs
extern void draw_tasks();


[[ noreturn ]]
static void sigtrap(int /*sig*/)
{
  dispatch_source_cancel(timer1);
  clrscr();
  printf("CTRL-C received, exiting program\n");
  exit(EXIT_SUCCESS);
}

[[ noreturn ]]
static void cancel_handler(void* timer_source)
{
  dispatch_release(reinterpret_cast<dispatch_source_t>(timer_source));
  dispatch_release(queue);
  printf("end\n");
  exit(0);
}

static void preemptor()
{
  if ((preempt_at_tick == 0) || (user_preemptor == nullptr) || (preemptor_active == true))
  {
    return;
  }

  if (current_tick_ >= preempt_at_tick)
  {
    preemptor_active = true;
    // run the user function
    user_preemptor(user_preemptor_data);
    // uninstall it when done
    preempt_at_tick = 0;
    user_preemptor = nullptr;
    preemptor_active = false;
  }
}

static void vector(void* /*timer*/)
{
  if (installed_timer_interrupt_in_service == true)
    return;
  installed_timer_interrupt_in_service = true;
  current_tick_++;
  preemptor();
  installed_timer_interrupt_in_service = false;
}


// Local functions

// sets the speed at which timer interrupts are made
static void set_timer_speed()
{
  // clamp timer_speed to be between 1 and 10000
  timer_speed = (timer_speed <= 0) ? 1 : ((timer_speed >= 10000) ? 10000 : timer_speed);
  dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 1000); // after 0.001 sec
  dispatch_source_set_timer(timer1, start, NSEC_PER_SEC / timer_speed, 0);  // 1/timer_speed sec  or  timer_speed Hz
}


// Implementation

// installs a user function that will get called at the given tick
static bool install_preempt_func(tick_t event_time, preemptor_t user_func, void* user_data)
{
  // if it's unsafe to install a user_func or one is already installed
  // then return false
  if ((user_func == nullptr) || (user_preemptor != nullptr) || (true == preemptor_active))
    return false;
  preemptor_active = true;
  preempt_at_tick = event_time;
  user_preemptor = user_func;
  user_preemptor_data = user_data;
  preemptor_active = false;
  return true;
}

// uninstalls an installed user function
static bool uninstall_preempt_func()
{
  // if it's unsafe to uninstall a user_func then return false
  if (preemptor_active == true)
    return false;
  preemptor_active = true;
  preempt_at_tick = 0;
  user_preemptor = nullptr;
  user_preemptor_data = nullptr;
  preemptor_active = false;
  return true;
}

// resumes timer so current_tick resumes updating from where it was suspended
static void resume_timer()
{
  dispatch_resume(timer1);
  timer_active = true;
}

// starts timer so that current_tick will automatically update
static void enable_timer()
{
  // catch ctrl-c
  signal(SIGINT, &sigtrap);
  // create timer queue
  queue = dispatch_queue_create("timerQueue", nullptr);
  // create dispatch timer source
  timer1 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
  // set handler for dispatch timer source for timer events
  dispatch_source_set_event_handler_f(timer1, vector);
  // set handler for when dispatch timer source is canceled
  dispatch_source_set_cancel_handler_f(timer1, cancel_handler);

  timer_speed = 1000;
  set_timer_speed();
  current_tick_ = 0;
  resume_timer();
}

// stops timer
static void disable_timer()
{
  timer_active = false;
  dispatch_suspend(timer1);
}

// suspends timer so current_tick can be temporarily made to stop updating
static void suspend_timer()
{
  disable_timer();
}

// speeds up timer so current_tick updates faster
static void speed_up_timer()
{
  timer_speed = timer_speed * 2;
  set_timer_speed();
}

// slows timer so current_tick updating more slowly
static void slow_down_timer()
{
  timer_speed = timer_speed / 2;
  set_timer_speed();
}

// delay() causes the computer to idle for the given number of ticks.
// It works by the fact that timer updates current_tick.
// The function could be re-written as a pre-empt routine, except this
// function couldn't be pre-empted because currently only one pre-empt
// function can be installed at a time.
// The pre-emptor should be reserved for use by the scheduler only.
void delay(ticks_t number_of_ticks, tick_t deadline)
{
  // must not call this if timer handler isn't installed
  if (timer_active == false)
  {
    printf("error: cannot call delay unless timer is enabled\n");
    exit(0);
  }

  tick_t finish_at = current_tick_ + number_of_ticks;
  if (deadline == 0)
    deadline = finish_at;

  if (current_tick_ > finish_at)
  {
    printf("error: overflow condition in delay\n");
    exit(0);
  }

  while (current_tick_ < finish_at)
  {
    // wait for next tick
    tick_t next_tick = current_tick_ + 1;

    // If enough slack to do some screen refresh
    if (current_tick_ + 40 < deadline)
    {
      // update the bar view of the tasks every tick
      draw_tasks();
    }

    while (current_tick_ < next_tick)
    {
      /* do nothing */

      // avoid busy loop when on hosted OS
      usleep(10);
    }
  }
}

tick_t current_tick()
{
  return current_tick_;
}

void set_current_tick(tick_t tick)
{
  current_tick_ = tick;
}

static
timer_driver_t macos_timer =
{
  "macos_timer",
  install_preempt_func,
  uninstall_preempt_func,
  enable_timer,
  disable_timer,
  suspend_timer,
  resume_timer,
  speed_up_timer,
  slow_down_timer
};

timer_driver_t timer;

void init_timer_driver()
{
  timer = macos_timer;
}

