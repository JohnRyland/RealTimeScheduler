/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

#define interrupt
typedef void interrupt (*isr_func_t)(...);

void outp(int,int);
void disable();
void enable();
isr_func_t getvect(int);
void setvect(int, isr_func_t);

