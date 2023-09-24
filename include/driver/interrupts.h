/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

typedef void (*func_t)();

struct interrupt_controller_vtable_t
{
  void (*setup_tables)();
  void (*remap_irqs)();
  void (*set_irq_mask)(uint64_t irq_mask);
  void (*register_handler)(uint16_t interrupt_number, func_t func);
  void (*enable)();
  void (*disable)();
};
