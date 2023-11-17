/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "compatibility.h"
#include "types/faults.h"

// handle faults
void k_fault_handler(fault_t fault);

// abort - something went wrong!
NO_RETURN
void k_critical_error(int code, const char* fmt, ...);

// dump an emergency screen with as much diagnostics as possible
extern "C"
NO_RETURN
void k_panic();

// restart the computer
NO_RETURN
void k_reboot();

NO_RETURN
void k_halt();

NO_RETURN
void exit(int);
