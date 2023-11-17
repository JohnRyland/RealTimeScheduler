/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "integers.h"

// Currently maps 1:1 for x86 exceptions, however this list is expected to
// be added to as new platform targets are added and each port needs to map
// to these enums. And where possible if it is essentially the same the same
// enum value is used where possible across targets.
enum class fault_t : uint8_t
{
  DIVISION_BY_ZERO,
  SINGLE_STEP_TRAP,
  NON_MASKABLE_INTERRUPT,
  DEBUG_BREAKPOINT,
  ARITHMETIC_OVERFLOW,
  BOUNDS_RANGE_EXCEEDED,
  INVALID_INSTRUCTION,
  COPROCESSOR_NOT_AVAILABLE,
  DOUBLE_FAULT,
  COPROCESSOR_SEGMENT_OVERRUN,
  INVALID_TASK_STATE_SEGMENT,  // is TSS x86 specific? perhaps more general term?
  SEGMENT_NOT_PRESENT,
  STACK_SEGMENT_FAULT,
  GENERAL_PROTECTION_FAULT,
  PAGE_FAULT,
  RESERVED_15,
  X87_FLOATING_POINT_EXCEPTION, // is this FP exceptions in general?
  ALIGNMENT_CHECK,
  MACHINE_CHECK,
  SIMD_FLOATING_POINT_EXCEPTION,  // should this just map to a general FP exception?
  VIRTUALIZATION_EXCEPTION,
  CONTROL_PROTECTION_EXCEPTION,
  RESERVED_22,
  RESERVED_23,
  RESERVED_24,
  RESERVED_25,
  RESERVED_26,
  RESERVED_27,
  RESERVED_28,
  RESERVED_29,
  RESERVED_30,
  RESERVED_31,
};
