/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include <stdarg.h>

enum log_level
{
  TRACE,
  DEBUG,
  NORMAL,
  SUCCESS,
  WARNING,
  ERROR,
  CRITICAL
};

enum log_system
{
  ALL,
  FORMATTER,
  STRING,
  LOGGER
};

// Use this before module initialization
void k_log_early(log_level level, const char* str);

// These can be used after modules initialized
void k_log_vfmt(log_level level, const char* fmt, va_list ap);

// This provides a kernel facility similar to printf
// I did consider calling this logf similar to printf, however
// this could be confused with the maths function for float logarithm,
// so elected to go with log_fmt with a k_ prefix to note it is
// a kernel only function.
void k_log_fmt(log_level level, const char* fmt, ...);

// Editing the enabled systems in the if statement can
// be used to turn on and off system specific debug logging.
// Note that some of this logging can be quite verbose and
// is only intended for low-level system debugging during
// development. I think this might be my preferred way of
// adding logging over using k_log_fmt because it includes
// the scope.
template <log_system system>
static inline constexpr
void k_dbg(const char* fmt, ...)
{
  if constexpr (
                // system == STRING ||
                // system == FORMATTER ||
                // system == LOGGER || 
                system == ALL )
  {
    va_list ap;
    va_start(ap, fmt);
    k_log_vfmt(DEBUG, fmt, ap);
    va_end(ap);
  }
}
