/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_TIMER_LINUX

#include "module/timer.h"
#include "module_manager.h"

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstdint>
#include <ctime>
#include <atomic>

#include <unistd.h>

//#include "timer.h"


// Global variables
static volatile tick_t current_tick_ = 0;   // number of ticks since timer was enabled
static unsigned timer_speed = 1000;
static std::atomic_bool timer_active(false);
static tick_t preempt_at_tick = 0;
static preemptor_t user_preemptor = nullptr;
static void* user_preemptor_data = nullptr;
static std::atomic_bool preemptor_active(false);
static std::atomic_bool installed_timer_interrupt_in_service(false);
static timer_t timerid;


// Externs
extern void draw_tasks();


static void block_timer()
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
  {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }
}

static void unblock_timer()
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
  {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }
}

[[ noreturn ]]
static void sigtrap(int /*sig*/)
{
  block_timer();
  clrscr();
  printf("CTRL-C received, exiting program\n");
  exit(EXIT_SUCCESS);
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

static void vector(int, siginfo_t*, void*)
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

  struct itimerspec its;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1000000000 / timer_speed;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
  {
    perror("timer_settime");
    exit(EXIT_FAILURE);
  }
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
  unblock_timer();
  //dispatch_resume(timer1);
  timer_active = true;
}

// starts timer so that current_tick will automatically update
static void enable_timer()
{
  // catch ctrl-c
  signal(SIGINT, &sigtrap);

  // Set up handler for timer
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = vector;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGRTMIN, &sa, NULL) == -1)
  {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  // Block the timer signal temporarily.
  block_timer();

  // Create the timer
  struct sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGRTMIN;
  sev.sigev_value.sival_ptr = &timerid;
  if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
  {
    perror("timer_create");
    exit(EXIT_FAILURE);
  }

  // Now allow the timer
  unblock_timer();

  timer_speed = 1000;
  set_timer_speed();
  current_tick_ = 0;
  resume_timer();
}

// stops timer
static void disable_timer()
{
  timer_active = false;
  block_timer();
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

void initialize_timer_driver()
{
  timer = macos_timer;
}

#endif // ENABLE_TIMER_LINUX
