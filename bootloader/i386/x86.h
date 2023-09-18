/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

typedef void (*isr_routine_t)(...);

int inportb(int port);
void outportb(int port, int val);
void enable();
void disable();
isr_routine_t getvect(int vec);
void setvect(int vec, isr_routine_t);

