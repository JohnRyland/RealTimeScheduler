/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

//#include "../../include/runtime/memory.h"
#include "kernel/module_manager.h"
#include "kernel/debug_logger.h"
#include "runtime/memory.h"
#include <config.h>

static
bool _modules_initialized = false;

static
module_t* module_class_head_ptrs[static_cast<size_t>(module_class::DRIVER_CLASS_COUNT)];

bool modules_initialized()
{
  return _modules_initialized;
}

void module_register(module_t& driver)
{
  module_t* head_ptr = module_class_head_ptrs[static_cast<size_t>(driver.type)];
  driver.next        = head_ptr;
  module_class_head_ptrs[static_cast<size_t>(driver.type)] = &driver;
}

// All potentially built-in module register functions
extern void register_cpu_intel_x86_module();
extern void register_cpu_generic_module();
extern void register_ethernet_rtl8139_driver();
extern void register_interrupt_generic_driver();
extern void register_interrupts_intel_8259_driver();
extern void register_keyboard_dos_driver();
extern void register_keyboard_intel_8048_driver();
extern void register_keyboard_termios_driver();
extern void register_keyboard_win32_driver();
extern void register_memory_generic_model();
extern void register_memory_x86_linear_model();
extern void register_memory_x86_virtual_model();
extern void register_random_intel_x86_device();
extern void register_random_generic_device();
extern void register_scheduler_realtime_module();
extern void register_ti_16650_uart_driver();
extern void register_text_dos_display();
extern void register_text_tty_display();
extern void register_text_vga_display();
extern void register_text_win32_display();
extern void register_timer_dos_module();
extern void register_timer_intel_8253_module();
extern void register_timer_linux_module();
extern void register_timer_macos_module();
extern void register_timer_win32_module();

void initialize_modules()
{
  mem_set(module_class_head_ptrs, 0, sizeof(module_class_head_ptrs));
  // can call register_module here for all the built-in drivers

# ifdef ENABLE_CPU_INTEL_X86
  register_cpu_intel_x86_module();
# endif
# ifdef ENABLE_CPU_GENERIC
  register_cpu_generic_module();
# endif

  //register_ethernet_rtl8139_driver();

# ifdef ENABLE_INTERRUPTS_GENERIC
  register_interrupt_generic_driver();
# endif
# ifdef ENABLE_INTERRUPTS_INTEL_8259
  register_interrupts_intel_8259_driver();
# endif
# ifdef ENABLE_KEYBOARD_DOS
  register_keyboard_dos_driver();
# endif
# ifdef ENABLE_KEYBOARD_INTEL_8048
  register_keyboard_intel_8048_driver();
# endif
# ifdef ENABLE_KEYBOARD_TERMIOS
  register_keyboard_termios_driver();
# endif
# ifdef ENABLE_KEYBOARD_WIN32
  register_keyboard_win32_driver();
# endif
# ifdef ENABLE_MEMORY_GENERIC
  register_memory_generic_model();
# endif
# ifdef ENABLE_MEMORY_X86_LINEAR
  register_memory_x86_linear_model();
# endif
# ifdef ENABLE_MEMORY_X86_VIRTUAL
  register_memory_x86_virtual_model();
# endif
# ifdef ENABLE_RANDOM_INTEL_X86
  register_random_intel_x86_device();
# endif
# ifdef ENABLE_RANDOM_GENERIC
  register_random_generic_device();
# endif
# ifdef ENABLE_SCHEDULER_REALTIME
  register_scheduler_realtime_module();
# endif
# ifdef ENABLE_SERIAL_16650
  register_ti_16650_uart_driver();
# endif
# ifdef ENABLE_TEXT_DOS
  register_text_dos_display();
# endif
# ifdef ENABLE_TEXT_TTY
  register_text_tty_display();
# endif
# ifdef ENABLE_TEXT_VGA
  register_text_vga_display();
# endif
# ifdef ENABLE_TEXT_WIN32
  register_text_win32_display();
# endif
# ifdef ENABLE_TIMER_DOS
  register_timer_dos_module();
# endif
# ifdef ENABLE_TIMER_INTEL_8253
  register_timer_intel_8253_module();
# endif
# ifdef ENABLE_TIMER_LINUX
  register_timer_linux_module();
# endif
# ifdef ENABLE_TIMER_MACOS
  register_timer_macos_module();
# endif
# ifdef ENABLE_TIMER_WIN32
  register_timer_win32_module();
# endif

  _modules_initialized = true;
}

module_t const* find_module_by_class(module_class driver_type)
{
  return module_class_head_ptrs[static_cast<size_t>(driver_type)];
}

//module_t const* find_driver_by_id(uint32_t id);
//module_t const* find_driver_by_name(const short_name& name);
//module_t const* find_driver_by_class_and_id(module_class driver_type, uint32_t id);
//module_t const* find_driver_by_class_and_name(module_class driver_type, const short_name& name);
