/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include <cstdarg>

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

// Use this before module initialization
void k_log_early(log_level level, const char* str);

// These can be used after modules initialized
void k_log_vfmt(log_level level, const char* fmt, va_list ap);
void k_log_fmt(log_level level, const char* fmt, ...);
