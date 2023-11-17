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
  KEYBOARD_DRIVER,         // handle key events
  TEXT_DISPLAY,            // display text on a terminal
  GRAPHICS_DISPLAY,        // display graphics on a screen
  DISK_DRIVER,             // low-level disk access
  RANDOM_DEVICE,           // generate random data

  // TODO create definitions
  IO_CONTROLLER,
  DMA_CONTROLLER,
  NETWORK_INTERFACE,
  NETWORK_PROTOCOL,
  VOLUME_MANAGER,          // mana
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
  // uint8_t     bus_type; ? address ?
  //uint16_t     vendor_id;
  //uint16_t     device_id;  // USB has product id
  uint32_t     id;
  short_name   name;
  module_t*    next;
  module_t*    prev;
  //bool       (*load)();
  //void       (*init)();
  //bool       (*unload)();
  void*        vtable;
  void*        instance;
  //module_class depends[8];
  // void*        details;
  // bool         loaded;
  // bool         initialized;
  // bool         enabled;
};

// static_assert(sizeof(module_t) == 64, "Fits in a cache line on 64-bit builds");

