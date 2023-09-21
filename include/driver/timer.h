/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include <stdint.h>

typedef uint32_t tick_t;
typedef uint32_t ticks_t;
typedef void (*preemptor_t)(void* user_data);

/*
 to guarantee a user pre-empt routine gets installed you can do something like:
     while (uninstall_timer_isr() != true)
       ; // do nothing
     while (install_timer_isr(event_time, user_func) != true)
       ; // do nothing
*/

void init_timer_driver();

void delay(ticks_t number_of_ticks, tick_t deadline = 0);

tick_t current_tick();
void set_current_tick(tick_t tick);

struct timer_driver_t
{
  const char* name;
  bool (*install_preemptor)(tick_t event_time, preemptor_t user_func, void* user_data);
  bool (*uninstall_preemptor)();
  void (*enable)();
  void (*disable)();
  void (*suspend)();
  void (*resume)();
  void (*speed_up)();
  void (*slow_down)();
  //void (*set_speed)(unsigned int rate);
};

//extern timer_driver_t timer;

#define timer   get_timer_ref()

timer_driver_t& get_timer_ref();


