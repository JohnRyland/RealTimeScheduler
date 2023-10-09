/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

typedef void (*isr_func_t)();

struct interrupt_controller_vtable_t
{
  void (*initialize)();
  void (*setup_tables)();
  void (*remap_irqs)();
  void (*set_irq_mask)(uint64_t irq_mask);
  void (*register_handler)(uint16_t interrupt_number, isr_func_t func);
  void (*enable)();
  void (*disable)();
};
