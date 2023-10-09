/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_TIMER_DOS

#include "module/timer.h"
#include "module_manager.h"

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include "interrupts.h"
#include "timer.h"



// Defines and constants
#define PIT_COMMAND    0x43
#define PIT_COUNTER_0  0x40
#define USER_TIMER_INT 0x08  // Could use INT 0x1C or INT 0x70 instead with a few modifications


// Global variables
volatile tick_t current_tick_;   // number of ticks since timer was enabled
void interrupt (*old_timer)(...);
volatile unsigned int timer_speed = 0x0000;
bool timer_not_installed_blocking = true;
tick_t preempt_at_tick = NULL;
isr_routine_t user_preempt_routine = NULL;
bool block_preemptor = false;
bool installed_timer_interrupt_in_service = false;


// Externs
extern void draw_tasks();


// Local functions

// sets the speed at which timer interupts are made
static void set_timer_speed(unsigned int rate)
{
  outp(PIT_COMMAND,0x36);
  outp(PIT_COUNTER_0,rate & 0x00FF);
  outp(PIT_COUNTER_0,(rate >> 8) & 0x00FF);
}

static void preemptor()
{
  if ((preempt_at_tick == 0) ||
	 (user_preempt_routine == NULL) || (block_preemptor == true))
    return;

  if (current_tick_ >= preempt_at_tick)
  {
    block_preemptor = true;
    // run the user function
    user_preempt_routine();
    // uninstall it when done
    preempt_at_tick = NULL;
    user_preempt_routine = NULL;
    block_preemptor = false;
  }
}

static void interrupt run_timer_isr(...)
{
  if (installed_timer_interrupt_in_service == true)
    return;
  installed_timer_interrupt_in_service = true;
  current_tick_++;
  preemptor();
  old_timer();
  installed_timer_interrupt_in_service = false;
}


// Implementation

// installs a user function that will get called at the given tick
bool install_timer_isr(tick_t event_time, void (*user_func)())
{
  // if it's unsafe to install a user_func or one is already installed
  // then return false
  if ((user_preempt_routine != NULL) || (block_preemptor == true))
    return false;
  block_preemptor = true;
  preempt_at_tick = event_time;
  user_preempt_routine = user_func;
  block_preemptor = false;
  return true;
}

// uninstalls an installed user function
bool uninstall_timer_isr()
{
  // if it's unsafe to uninstall a user_func then return false
  if (block_preemptor == true)
    return false;
  block_preemptor = true;
  preempt_at_tick = NULL;
  user_preempt_routine = NULL;
  block_preemptor = false;
  return true;
}

// starts timer so that current_tick will automatically update
void enable_timer()
{
  disable();
  old_timer = getvect(USER_TIMER_INT);
  setvect(USER_TIMER_INT, run_timer_isr);
  timer_speed = 0x0400;
  set_timer_speed(timer_speed);    // approx 1000Hz
  current_tick_ = 0;
  enable();
  timer_not_installed_blocking = false;
}

// stops timer
void disable_timer()
{
  timer_not_installed_blocking = true;
  disable();
  timer_speed = 0x0000;
  set_timer_speed(timer_speed);    // default 18.2Hz
  setvect(USER_TIMER_INT, old_timer);
  enable();
}

// suspends timer so current_tick can be temporarily made to stop updating
void suspend_timer()
{
  disable_timer();
}

// resumes timer so current_tick resumes updating from where it was suspended
void resume_timer()
{
  disable();
  old_timer = getvect(USER_TIMER_INT);
  setvect(USER_TIMER_INT, run_timer_isr);
  timer_speed = 0x0400;
  set_timer_speed(timer_speed);
  enable();
  timer_not_installed_blocking = false;
}

// speeds up timer so current_tick updates faster
void speed_up_timer()
{
  disable();
  timer_speed = timer_speed * 2;
  set_timer_speed(timer_speed);
  enable();
}

// slows timer so current_tick updating more slowly
void slow_down_timer()
{
  disable();
  if (timer_speed == 0)
    timer_speed = -1;
  timer_speed = timer_speed / 2;
  set_timer_speed(timer_speed);
  enable();
}

// delay() causes the computer to idle for the given number of ticks.
// It works by the fact that timer updates current_tick.
// The function could be re-written as a pre-empt routine, except this
// function couldn't be pre-empted because currently only one pre-empt
// function can be installed at a time.
// The pre-emptor should be reserved for use by the scheduler only.
void delay(ticks_t number_of_ticks)
{
  // must not call this if my_timer handler isn't installed
  if (timer_not_installed_blocking == true)
  {
    printf("error: cannot call delay unless timer is enabled\n");
    exit(0);
  }

  tick_t finish_at = current_tick_ + number_of_ticks;

  if (current_tick_ > finish_at)
  {
    printf("error: overflow condition in delay\n");
    exit(0);
  }

  while (current_tick_ < finish_at)
  {
    // wait for next tick
    tick_t next_tick = current_tick_ + 1;
    while (current_tick_ < next_tick)
      /* do nothing */ ;

    // update the bar view of the tasks every tick
    draw_tasks();
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

#endif // ENABLE_TIMER_DOS
