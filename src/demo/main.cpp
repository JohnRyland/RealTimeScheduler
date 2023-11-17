/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#include "common/formatter.hpp"
#include "conio.h"
#include "exception_handler.h"
#include "helpers.h"
#include "kernel/schedule.h"
#include "kernel/task_manager.h"
#include "kernel/debug_logger.h"
#include "kernel/module_manager.h"
#include "module/serial.h"
#include "types/modules.h"

#include "common/logger.hpp"


static
void repeat_putch(char ch, int repeats)
{
  for (int i = 0; i < repeats; ++i)
    putch(ch);
}

static
void draw_frame_row(unsigned x, unsigned y, char ch1, char ch2, char ch3, char ch4)
{
  gotoxy(x, y);
  putch(ch1);
  repeat_putch('\xc4', 25);
  putch(ch2);
  repeat_putch('\xc4', 25);
  putch(ch3);
  repeat_putch('\xc4', 25);
  putch(ch4);
}

static
void draw_bar_row(unsigned x, unsigned y, char ch1)
{
  gotoxy(x, y);
  repeat_putch('\xc4', 19);
  putch(ch1);
  repeat_putch('\xc4', 59);
}

// Misc
extern void initialize_screen();
extern void initialize_banner();
extern void initialize_conio();
extern void initialize_vesa();
extern void initialize_acpi();
extern void initialize_pcie();
extern void k_sleep(int);
extern void k_power_off();

static
void initialize_serial_ports()
{
  int i = 0;
  const char* name[] = { "COM1", "COM2", "COM3", "COM4" };
  const module_t* serial = find_module_by_class(module_class::SERIAL_DRIVER);
  while (serial)
  {
    ((serial_driver_vtable_t*)(serial->vtable))->initialize((serial_driver_t*)serial->instance,
        baud_rate_t::BAUD_19200, word_size_t::WORD_SIZE_8_BITS, stop_bits_t::ONE_STOP_BIT, parity_t::NO_PARITY);

    ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, name[i][3]);

    serial = serial->next;
    i++;
  }
}

void log_test(bool early = false)
{
  if (early)
  {
    k_log_early(TRACE, "trace");
    k_log_early(DEBUG, "debug");
    k_log_early(NORMAL, "normal");
    k_log_early(SUCCESS, "success");
    k_log_early(WARNING, "warning");
    k_log_early(ERROR, "error");
    k_log_early(CRITICAL, "critical");
  }
  else
  {
    k_log_fmt(TRACE, "trace\n");
    k_log_fmt(DEBUG, "debug\n");
    k_log_fmt(NORMAL, "normal\n");
    k_log_fmt(SUCCESS, "success\n");
    k_log_fmt(WARNING, "warning\n");
    k_log_fmt(ERROR, "error\n");
    k_log_fmt(CRITICAL, "critical\n");  
  }
}

bool flush_key_event()
{
  // Get all key events for a short period
  for (int i = 0; i < 0x40000; i++)
  {
    if (kbhit())
    {
      getch();
      return true;
    }
  }
  return false;
}

static
void wait_for_keypress()
{
  // purge any pending keys
  while (flush_key_event());

  // wait for a bit longer for any key presses
  for (int i = 0; i < 0x600000; i++)
    if (kbhit()) {
      flush_key_event();
      break;
    }

  // purge any extra events
//  while (flush_key_event());
}

static
void show_info()
{
  // draw a title screen with instructions
  clrscr();
  initialize_banner();
  k_log_fmt(SUCCESS, "\n                               MagicCore RTOS v1.0\n");
  k_log_fmt(DEBUG, "                           Copyright (C) 2023, John Ryland\n\n");
  k_log_fmt(NORMAL, "\nKeyboard functions:\n");
  k_log_fmt(NORMAL, "  'Q' 'X' or 'ESC' all exit the program immediately\n");
  k_log_fmt(NORMAL, "  ' ' SPACE BAR pauses everything until a key is pressed\n");
  k_log_fmt(NORMAL, "  '+' '=' or UP ARROW speeds things up\n");
  k_log_fmt(NORMAL, "  '-' or DOWN ARROW slows things up\n");
  k_log_fmt(NORMAL, "\nMeaning of characters in display:\n");
  k_log_fmt(NORMAL, "  '\xb3' start_not_before\n");
  k_log_fmt(NORMAL, "  '\xba' complete_not_after\n");
  k_log_fmt(NORMAL, "  '\xdb\xdb\xdb\xdb\xdb' where it is anticipated it will run\n");
  k_log_fmt(NORMAL, "\nPress any key to continue.\n");
  wait_for_keypress();
}

extern
void putPixel(int x, int y, uint16_t r, uint16_t g, uint16_t b);

static
void draw_vline(int x, int y1, int y2)
{
  for (int i = y1*16; i < y2*16; i++)
    putPixel(x*8 - 4, i - 5, 0xff, 0xff, 0xff);
}

static
void draw_hline(int x1, int x2, int y)
{
  for (int i = x1*8; i < x2*8; i++)
    putPixel(i - 4, y*16 - 5, 0xff, 0xff, 0xff);
}

static
void draw_graphic_frames()
{
  // draw borders and windows with line drawing graphics
  draw_vline(1, 1, 37);
  draw_vline(27, 13, 37);
  draw_vline(53, 1, 37);
  draw_vline(79, 1, 37);
  draw_hline(1, 79, 1);
  gotoxy(2, 2);
  k_log_fmt(NORMAL, "Status Window");
  draw_hline(1, 79, 3);
  draw_hline(1, 79, 13);
  draw_hline(1, 79, 15);
  draw_hline(1, 79, 25);
  draw_hline(1, 79, 27);
  draw_hline(1, 79, 37);
  gotoxy(40, 38);
  k_log_fmt(NORMAL, "Schedule");
  draw_hline(1, 80, 39);
  draw_vline(20, 40, 41 + 8);
  for (unsigned i = 0; i < 8; i += 2)
    draw_hline(1, 80, 41 + i);
  draw_hline(1, 80, 49);
}

static
void draw_text_frames()
{
  // draw borders and windows with line drawing characters
  for (unsigned i = 1; i < 37; i++)
  {
    gotoxy(1, i); putch('\xb3');
    if (i > 13)
    {
      gotoxy(27, i); putch('\xb3');
    }
    gotoxy(53, i); putch('\xb3');
    gotoxy(79, i); putch('\xb3');
  }
  
  draw_frame_row(1,  1, '\xda', '\xc4', '\xc2', '\xbf');
  gotoxy(2, 2);
  k_log_fmt(NORMAL, "Status Window");
  draw_frame_row(1,  3, '\xc3', '\xc4', '\xc5', '\xb4');
  draw_frame_row(1, 13, '\xc3', '\xc2', '\xc5', '\xb4');
  draw_frame_row(1, 15, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 25, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 27, '\xc3', '\xc5', '\xc5', '\xb4');
  draw_frame_row(1, 37, '\xc0', '\xc1', '\xc1', '\xd9');

  gotoxy(40, 38);
  k_log_fmt(NORMAL, "Schedule");
  draw_bar_row(1, 39, '\xc2');
  for (unsigned i = 0; i < 8; i += 2)
  {
    gotoxy(1, 40 + i); repeat_putch(' ', 19); putch('\xb3');
    draw_bar_row(1, 41 + i, '\xc5');
  }
  gotoxy(1, 48); repeat_putch(' ', 19); putch('\xb3');
  draw_bar_row(1, 49, '\xc1');
}

static int load_modules = 1;       // "modules" | "no_modules"
static int graphics = 1;           // "vga" | "no_vga"
static int quiet = 0;              // "quiet"
static int hosted = 0;             // "hosted"  (running as a guest OS on some already hosted environment)
static int no_args = 0;            // " "
static const char* boot_entry = "none";

struct arg_desc_t
{
  const char* param_str;
  int*        param_addr;
  int         param_val;
};

static const arg_desc_t arg_descs[] =
{
  { "modules",    &load_modules, 1 },
  { "no_modules", &load_modules, 0 },
  { "vga",        &graphics,     1 },
  { "no_vga",     &graphics,     0 },
  { "quiet",      &quiet,        1 },
  { "hosted",     &hosted,       1 },
  { " ",          &no_args,      1 }
};

static
void parse_arg_span(const char* arg_start, const char* arg_end)
{
  bool found = false;
  for (int i = 0; i < 7 && !found; i++)
  {
    if (!mem_cmp(arg_descs[i].param_str, arg_start, arg_end - arg_start))
    {
      // k_log_early(SUCCESS, arg_descs[i].param_str);
      *arg_descs[i].param_addr = arg_descs[i].param_val;
      found = true;
    }
  }
  if (!found)
  {
    k_log_early(ERROR, "[ ] Unhandled kernel parameter");
    k_log_early(ERROR, arg_start);
  }
}

static
void parse_arg_str(const char* arg)
{
  for (const char* arg_start = arg; *arg; arg++)
    if (*arg == ' ' && arg_start != arg)
    {
      parse_arg_span(arg_start, arg);
      while (*arg && *arg == ' ')
        arg++;
      arg_start = arg;
      arg--;
    }
}

static
void initialize_parameters(int argc, const char* argv[])
{
  if (argc > 0)
  {
    boot_entry = argv[0];

    // for each arg, parse the args
    for (int i = 1; i < argc; i++)
      parse_arg_str(argv[i]);
  }
}

void k_log_main(log_level lvl, const char* msg)
{
  if (lvl >= log_level::WARNING || !quiet)
    k_log_early(lvl, msg);
}

void sysLog(const char* str, size_t /*len*/)
{
//  k_log_early(ERROR, str);
  k_log_fmt(WARNING, str);
}

void sysAbort()
{
  //k_halt();
  k_power_off();
}

void sysFatal(const char* str)
{
  k_log_fmt(ERROR, str);
  k_power_off();
//  k_halt();
//  k_panic();
}

extern void dump_memorymap();


extern "C"
int __main(int argc, const char* argv[])
{
  initialize_parameters(argc, argv);
  k_log_main(SUCCESS, "[X] Initialized parameters.");
  initialize_modules();
  k_log_main(SUCCESS, "[X] Initialized modules.");
  initialize_serial_ports();
  k_log_main(SUCCESS, "[X] Initialized serial ports.");
  initialize_conio();
  k_log_main(SUCCESS, "[X] Initialized console.");
  initialize_vesa();
  k_log_main(SUCCESS, "[X] Initialized video.");
  textmode(C4350);  // change screen mode
  k_log_fmt(SUCCESS, "[X] Changed screen mode.\n");
  initialize_acpi();
  k_log_fmt(SUCCESS, "[X] Initialized ACPI.\n");
  initialize_pcie();
  k_log_fmt(SUCCESS, "[X] Initialized PCIE.\n");
  dump_memorymap();
  k_log_fmt(SUCCESS, "[X] Dumped memory map.\n");

/*
  sysLog("this is a sys log\n", 0);
  LogDebug("this is a log debug\n");
  LogWarning("this is a log warning\n");
//  LogError("this is a log error\n");  
  LogWarning("this is {2} test {1} \n", "message", 1);
  LogDebug("this is {2} test {1} \n", "message", 1);
  LogWarning("this is {2} test {1} \n", "message", 1);
//  LogError("this is {2} test {1} \n", "message", 1);
  // log_test();
  // k_halt();
  // k_power_off();
*/

  if (!quiet)
  {
    k_log_early(SUCCESS, "Press any key to continue.");
    wait_for_keypress();
  }

/*  
  FACP  - Fixed ACPI Description Table - standard ACPI flags and fields, some power management and general stuff
  APIC  - Multiple APIC Description Table (MADT) Format - Supporting APIC would probably be needed to support multi-core with interrupts
  HPET  - High Precision Event Timer Table - Basic idea is up to 32 values can be programmed to trigger IRQ when TSC or similar gets to that value
  MCFG  - PCI Express Memory-mapped Configuration - PCI-e enumeration (https://wiki.osdev.org/PCI_Express)
  WAET  - Windows ACPI Emulated Devices Table - no idea, looks like if windows is guest or host which neither is the case so don't care
*/

  // enables interrupts
  start_timer();

  if (!quiet)
    show_info();

  if (graphics)
    initialize_screen();
  else
    textmode(C4350);  // change screen mode

  k_log_main(SUCCESS, "[X] Initialized screen.");
  clrscr();
  gotoxy(0, 0);
  k_log_fmt(SUCCESS, "                               MagicCore RTOS v1.0");

  if (graphics)
    draw_graphic_frames();
  else
    draw_text_frames();

  initialize_tasks();
  initialize_scheduler();
  initialize_status();
  initialize_timer_driver();

  // draw_tasks();  // should make this a task

  //                                                                       wait for   start after
  //                                                                     id   \       /  bound  complete before           period
  //                                                                       \  |      |    |      |                         |
  // this task must be here as it needs to be the first one to run
  status_to_adding_a_task(request_to_add_task(online_scheduler,            0, 0,     0,   5,     0, UPDATE_SCHEDULE_RATE - 1,   "Online Scheduler", 54, 2), "online scheduler");

  // add user tasks to be scheduled
  status_to_adding_a_task(request_to_add_task(test_deterministic,         10, 0,     0, 100,     0,                       50,     "unaccept test1",  2, 14), "task with exec_bound > period");
  status_to_adding_a_task(request_to_add_task(draw_tasks,                  1, 0,     0,  10,     0,                       50, "Visualize Schedule",  2, 14), "visualize schedule");
  // This task shouldn't be accepted because the exec_bound of 50 can't be added between draw_tasks tasks which are every 50 ticks
  status_to_adding_a_task(request_to_add_task(test_deterministic,          2, 0,     0,   5,     0,                      200,      "Deterministic", 28, 14), "deterministic");
  status_to_adding_a_task(request_to_add_task(test_exponential,            3, 0,     0,  10,     0,                      700,        "Exponential", 54, 14), "exponential");
  status_to_adding_a_task(request_to_add_task(test_binary,                 4, 0,     0,  10,     0,                      500,             "Binary",  2, 26), "binary");
  status_to_adding_a_task(request_to_add_task(test_adding_task_on_the_fly, 5, 0, 10000,   5, 10500,                        0,  "Exec another task", 28, 26), "on the fly task");

  start_timer();

  // set the realtime system going
  run_on_line_scheduler();
  return 0;
}

