/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

struct ethernet_controller_vtable_t
{
  void (*initialize)();
  void (*enable)();
  void (*disable)();
  void (*reset)();
  void (*send_packet)();
  void (*receive_packet)();
  void (*handle_interrupt)();
};
