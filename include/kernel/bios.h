/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

extern "C" void reboot();

extern void call_real_mode_interrupt(int interrupt, uint32_t eax, uint32_t ebx = 0,
                              uint32_t ecx = 0, uint32_t edx = 0, uint32_t esi = 0,
                              uint32_t edi = 0, uint32_t  es = 0, uint32_t  ds = 0);
