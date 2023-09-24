/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#include "../../include/driver/driver.h"
#include "../../include/runtime/memory.h"

static
driver_t* driver_class_head_ptrs[static_cast<size_t>(driver_class::DRIVER_CLASS_COUNT)];

void register_driver(driver_t& driver)
{
  driver_t* head_ptr = driver_class_head_ptrs[static_cast<size_t>(driver.type)];
  driver.next        = head_ptr;
  driver_class_head_ptrs[static_cast<size_t>(driver.type)] = &driver;
}

extern void register_termios_driver();
extern void register_tty_driver();
extern void register_ti_16650_uart_driver();

void initialize_drivers()
{
  memset(driver_class_head_ptrs, 0, sizeof(driver_class_head_ptrs));
  // can call register_driver here for all the built-in drivers

  register_termios_driver();
  register_tty_driver();
  register_ti_16650_uart_driver();

  // register_driver(keybaord_driver);
  // register_driver(text_driver);
  // register_driver(timer_driver);
}

driver_t const* find_driver_by_class(driver_class driver_type)
{
  return driver_class_head_ptrs[static_cast<size_t>(driver_type)];
}

//driver_t const* find_driver_by_id(uint32_t id);
//driver_t const* find_driver_by_name(const short_name& name);
//driver_t const* find_driver_by_class_and_id(driver_class driver_type, uint32_t id);
//driver_t const* find_driver_by_class_and_name(driver_class driver_type, const short_name& name);
