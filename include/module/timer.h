/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"
#include "interrupts.h"

enum class timer_type_t : uint8_t
{
  SINGLE_SHOT,
  PERIODIC,
};

struct timer_t;
typedef void (*func_t)();

struct timer_vtable_t
{
  bool (*initialize)(timer_t* driver, interrupt_controller_vtable_t& interrupt_controller);
  bool (*set_speed)(timer_t* driver, uint32_t speed);
  bool (*enable)(timer_t* driver);
  uint64_t (*current_tick)(timer_t* driver);
  uint64_t (*create_timer)(timer_t* driver, timer_type_t type, uint64_t first_tick, uint64_t ticks_between, func_t callback, timer_t& timer);
  uint64_t (*cancel_timer)(timer_t* driver, timer_t& timer);
};


#if 1

typedef void (*preemptor_t)(void* user_data);

/*
 to guarantee a user pre-empt routine gets installed you can do something like:
     while (uninstall_timer_isr() != true)
       ; // do nothing
     while (install_timer_isr(event_time, user_func) != true)
       ; // do nothing
*/

void start_timer();
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

#endif
