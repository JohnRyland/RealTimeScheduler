/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "integers.h"
#include "strings.h"

enum class module_class : uint8_t
{
  // Have some initial definitions
  // in order of required
  CPU_STATE,               // misc CPU functions
  MEMORY_MODEL,            // low-level memory manager
  SERIAL_DRIVER,           // early stage debugging
  INTERRUPT_CONTROLLER,    // required for timer
  TIMER_DRIVER,            // required for scheduler
  SCHEDULER_MODULE,        // required to run tasks
  KEYBOARD_DRIVER,
  TEXT_DISPLAY,
  GRAPHICS_DISPLAY,
  RANDOM_DEVICE,

  // TODO create definitions
  IO_CONTROLLER,
  DMA_CONTROLLER,
  NETWORK_INTERFACE,
  NETWORK_PROTOCOL,
  DISK_DRIVER,
  FILE_SYSTEM,

  DRIVER_CLASS_COUNT
};

struct module_vtable_t
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
struct module_t
{
  module_class type;
  uint32_t     id;
  short_name   name;
  module_t*    next;
  module_t*    prev;
  //bool       (*load)();
  //void       (*init)();
  //bool       (*unload)();
  void*        vtable;
  void*        instance;
  // module_class depends;
  // void*        details;
  // bool         loaded;
  // bool         initialized;
  // bool         enabled;
};

// static_assert(sizeof(module_t) == 64, "Fits in a cache line on 64-bit builds");

