/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

enum class cpu_register_t : uint8_t
{
  CR0,
  CR1,
  CR2,
  CR3,
  /* ... */
  TSC
};

struct cpu_state_vtable_t
{
  void (*initialize)();
  void (*halt)();         // make the cpu idle until next interrupt

  void (*enable_interrupts)();
  void (*disable_interrupts)();

  uint8_t (*inport_byte)(uint32_t port);
  uint16_t (*inport_word)(uint32_t port);
  uint32_t (*inport_dword)(uint32_t port);

  void (*outport_byte)(uint32_t port, uint8_t byte);
  void (*outport_word)(uint32_t port, uint16_t word);
  void (*outport_dword)(uint32_t port, uint32_t dword);

  uint64_t (*read_cpu_register)(cpu_register_t reg);
  void (*write_cpu_register)(cpu_register_t reg, uint64_t value);
};
