/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "../runtime/types.h"

enum class driver_class : uint8_t
{
  // defined
  GRAPHICS_DISPLAY,
  INTERRUPT_CONTROLLER,
  KEYBOARD_DRIVER,
  RANDOM_DEVICE,
  SERIAL_DRIVER,
  TEXT_DISPLAY,
  TIMER_DRIVER,

  // todo
  IO_CONTROLLER,
  DMA_CONTROLLER,
  NETWORK_INTERFACE,
  NETWORK_PROTOCOL,
  DISK_DRIVER,
  FILE_SYSTEM,

  DRIVER_CLASS_COUNT
};

struct driver_vtable_t
{
  //bool tmp;
  //bool (*load)();
  //bool (*unload)();
  // void (&dump)();
  // bool (&initialize)();
  // bool (&deinitialize)();
  // void (&enable)();
  // void (&disable)();
};

/// Base for a specific type of driver
struct driver_t
{
  driver_class type;
  uint32_t     id;
  short_name   name;
  driver_t*    next;
  driver_t*    prev;
  bool       (*load)();
  bool       (*unload)();
  void*        vtable;
  // void*        details;
  // bool         loaded;
  // bool         initialized;
  // bool         enabled;
};

// static_assert(sizeof(driver_t) == 64, "Fits in a cache line on 64-bit
// builds");

void initialize_drivers();
void register_driver(driver_t& driver);

driver_t const* find_driver_by_class(driver_class driver_type);
// driver_t const* find_driver_by_id(uint32_t id);
// driver_t const* find_driver_by_name(const short_name& name);
// driver_t const* find_driver_by_class_and_id(driver_class driver_type,
// uint32_t id); driver_t const* find_driver_by_class_and_name(driver_class
// driver_type, const short_name& name);
