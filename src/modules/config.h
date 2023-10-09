/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

// Enable all here, however the build system will put another config.h
// earlier in the include path so that a more specific one will be used.
#define ENABLE_CPU_INTEL_X86
#define ENABLE_CPU_GENERIC
#define ENABLE_INTERRUPTS_GENERIC
#define ENABLE_INTERRUPTS_INTEL_8259
#define ENABLE_KEYBOARD_DOS
#define ENABLE_KEYBOARD_INTEL_8048
#define ENABLE_KEYBOARD_TERMIOS
#define ENABLE_KEYBOARD_WIN32
#define ENABLE_MEMORY_GENERIC
#define ENABLE_MEMORY_X86_LINEAR
#define ENABLE_MEMORY_X86_VIRTUAL
#define ENABLE_RANDOM_INTEL_X86
#define ENABLE_RANDOM_GENERIC
#define ENABLE_SCHEDULER_REALTIME
#define ENABLE_SERIAL_16650
#define ENABLE_TEXT_DOS
#define ENABLE_TEXT_TTY
#define ENABLE_TEXT_VGA
#define ENABLE_TEXT_WIN32
#define ENABLE_TIMER_DOS
#define ENABLE_TIMER_INTEL_8253
#define ENABLE_TIMER_LINUX
#define ENABLE_TIMER_MACOS
#define ENABLE_TIMER_WIN32
