/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_INTERRUPTS_GENERIC

#include "module/interrupts.h"
#include "module_manager.h"

static
void initialize()
{
}

static
void setup_tables()
{
}

static
void remap_irqs()
{
}

static
void set_irq_mask(uint64_t /*irq_mask*/)
{
}

static
void register_handler(uint16_t /*interrupt_number*/, isr_func_t /*func*/)
{
}

static
void enable_interrupts()
{
}

static
void disable_interrupts()
{
}

static
interrupt_controller_vtable_t interrupt_generic_vtable =
{
  .initialize         = initialize,
  .setup_tables       = setup_tables,
  .remap_irqs         = remap_irqs,
  .set_irq_mask       = set_irq_mask,
  .register_handler   = register_handler,
  .enable             = enable_interrupts,
  .disable            = disable_interrupts,
};

static
module_t interrupt_generic_module = 
{
  .type    = module_class::INTERRUPT_CONTROLLER,
  .id      = 0x12024, // TODO: how to assign these? during register?
  .name    = { "interrupt_gen" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &interrupt_generic_vtable,
};

void register_interrupt_generic_driver()
{
  module_register(interrupt_generic_module);
}

#endif // ENABLE_INTERRUPTS_GENERIC
