/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_CPU_GENERIC

#include "module/cpu.h"
#include "module_manager.h"
#include "setjmp.h"

static
void initialize()
{
}

static
void halt()
{
  for(;;)
    /* do nothing */;
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
uint8_t inport_byte(uint32_t /*port*/)
{
  return 0;
}

static
uint16_t inport_word(uint32_t /*port*/)
{
  return 0;
}

static
uint32_t inport_dword(uint32_t /*port*/)
{
  return 0;
}

static
void outport_byte(uint32_t /*port*/, uint8_t /*byte*/)
{
}

static
void outport_word(uint32_t /*port*/, uint16_t /*word*/)
{
}

static
void outport_dword(uint32_t /*port*/, uint32_t /*dword*/)
{
}

static
uint64_t read_cpu_register(cpu_register_t /*reg*/)
{
  return 0;
}

static
void write_cpu_register(cpu_register_t /*reg*/, uint64_t /*value*/)
{
}

static
cpu_state_vtable_t cpu_state_vtable =
{
  .initialize         = initialize,
  .halt               = halt,
  .enable_interrupts  = enable_interrupts,
  .disable_interrupts = disable_interrupts,
  .inport_byte        = inport_byte,
  .inport_word        = inport_word,
  .inport_dword       = inport_dword,
  .outport_byte       = outport_byte,
  .outport_word       = outport_word,
  .outport_dword      = outport_dword,
  .read_cpu_register  = read_cpu_register,
  .write_cpu_register = write_cpu_register,
};

static
module_t cpu_generic_module = 
{
  .type    = module_class::CPU_STATE,
  .id      = 0x12024, // TODO: how to assign these? during register?
  .name    = { "cpu_generic" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &cpu_state_vtable,
};

void register_cpu_generic_module()
{
  module_register(cpu_generic_module);
}

#endif // ENABLE_CPU_GENERIC
