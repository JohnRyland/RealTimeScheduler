/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "timer.h"
#include "runtime.h"
#include "constants.h"
#include "x86.h"
#include "helpers.h"

// Variables
static volatile tick_t current_tick_;   // number of ticks since timer was enabled
static volatile unsigned int timer_speed = 0x0000;
static bool timer_not_installed_blocking = true;
static tick_t preempt_at_tick = 0;
static preemptor_t user_preempt_routine = nullptr;
static void* user_preemptor_data = nullptr;
static bool block_preemptor = false;
static bool installed_timer_interrupt_in_service = false;

// Current text position
static
int curX;
static
int curY;

unsigned random(unsigned)
{
  // TODO
  return 0;
}

void clrscr()
{
  memset((void*)VGA_TEXT_BASE, 0, VGA_TEXT_SIZE);
}

void gotoxy(unsigned x, unsigned y)
{
  curX = x;
  curY = y;
}

void textmode(int mode)
{
  // do nothing
}

int getch()
{
  while (!kbhit())
    /* wait */;
  unsigned int ch = (unsigned char)inportb(PPI_DATA); // read the key that is pressed

  // Rough keymap
  static const char keymap[] = " \0331234567890-=\b\tqwertyuiop[]\r asdfghjkl;'~ \nzxcvbnm,./ **               789-456+1230 ";

  bool press = ch <= 127;
  int mapped = (press) ? ch : (ch - 128);
  if (mapped < sizeof(keymap))
    mapped = keymap[mapped];

  /*
  print_str(" *** Key: ");
  print_int(ch);
  print_str(" mapped to: ");
  putch(mapped);
  if (press)
    print_str(" pressed *** ");
  else
    print_str(" released *** ");
  */

  if (press)
    return mapped;

  return -1;
}

void putch(char ch)
{
  curX++;
  curY = curY % 51;
  curX = curX % 81;
  if (ch == '\n')
    curX = 0, curY++;
  else
    ((short*)VGA_TEXT_BASE)[curY*80 + curX - 1] = 0x0700 | (unsigned)((unsigned char)ch);
}


bool kbhit()
{
  // check if a key is waiting
  int status = inportb(PPI_STATUS);
  if (!(status & 1))    // check output buffer
    return false;
  if ((status & 0x20))  // check if PS2 mouse byte
    return false;
  return true;
}

void puts2(const char* str)
{
  while (*str)
    putch(*str++);
  putch('\n');
}

// sets the speed at which timer interupts are made
static
void set_timer_speed(unsigned int rate)
{
  outportb(PIT_COMMAND, 0x36);
  outportb(PIT_CHANNEL_0, rate & 0x00FF);
  outportb(PIT_CHANNEL_0, (rate >> 8) & 0x00FF);
}

static
void preemptor()
{
  if ((preempt_at_tick == 0) ||
	 (user_preempt_routine == nullptr) || (block_preemptor == true))
    return;

  if (current_tick_ >= preempt_at_tick)
  {
    block_preemptor = true;
    // run the user function
    user_preempt_routine(user_preemptor_data);
    // uninstall it when done
    preempt_at_tick = 0;
    user_preempt_routine = nullptr;
    block_preemptor = false;
  }
}

static
void run_timer_isr(...)
{
  if (installed_timer_interrupt_in_service == true)
    return;
  installed_timer_interrupt_in_service = true;
  current_tick_++;
  preemptor();
  installed_timer_interrupt_in_service = false;
}

static
void keyboard_handler()
{
  ((char*)VGA_TEXT_BASE)[0] = ((char*)VGA_TEXT_BASE)[0] == '^'  ? 'v'  : '^';
  ((char*)VGA_TEXT_BASE)[1] = ((char*)VGA_TEXT_BASE)[1] == 0x0B ? 0xB0 : 0x0B;
}

extern "C"
void interrupt_handler(uint32_t interruptNumberOrMask)
{
  if (interruptNumberOrMask < 0x20)
  {
    puts2(" ** CPU fault ** ");
    print_int(interruptNumberOrMask);
    while (true)
     ;
    exit(-interruptNumberOrMask);
  }

  outportb(0x20, 0x0b);
  if (!inportb(0x20))  // get the in-service register
  {
    puts2(" **SPURIOUS** ");
    return;  // It was a spurious interrupt, ignore
  }

  switch (interruptNumberOrMask)
  {
    case 32: run_timer_isr(); break;
    case 33: keyboard_handler(); break;
    default:
      puts2(" **OTHER** ");
  }

  outportb(0x20, 0x20); // Send EOI (End-of-interrupt)
}

extern "C"
void _interrupt_handler(uint32_t interruptNumberOrMask)
{
  interrupt_handler(interruptNumberOrMask);
}

// installs a user function that will get called at the given tick
static
bool install_timer_isr(tick_t event_time, preemptor_t user_func, void* user_data)
{
  // if it's unsafe to install a user_func or one is already installed
  // then return false
  if ((user_func == nullptr) || (user_preempt_routine != nullptr) || (block_preemptor == true))
    return false;
  block_preemptor = true;
  preempt_at_tick = event_time;
  user_preempt_routine = user_func;
  user_preemptor_data = user_data;
  block_preemptor = false;
  return true;
}

// uninstalls an installed user function
static
bool uninstall_timer_isr()
{
  // if it's unsafe to uninstall a user_func then return false
  if (block_preemptor == true)
    return false;
  block_preemptor = true;
  preempt_at_tick = 0;
  user_preempt_routine = nullptr;
  block_preemptor = false;
  return true;
}

// starts timer so that current_tick will automatically update
void enable_timer()
{
  //puts2(" -- enable -- ");
  disable();
  //getvect(USER_TIMER_INT);
  //setvect(USER_TIMER_INT, run_timer_isr);
  //timer_speed = 0x4000;
  set_timer_speed(timer_speed);    // approx 1000Hz
  current_tick_ = 0;
  timer_not_installed_blocking = false;
  enable();
}

// stops timer
void disable_timer()
{
  //puts2(" -- disable -- ");
  disable();
  timer_not_installed_blocking = true;
  set_timer_speed(0x0000);    // default 18.2Hz
  enable();
}

// suspends timer so current_tick can be temporarily made to stop updating
void suspend_timer()
{
  //puts2(" -- suspend -- ");
  disable_timer();
}

// resumes timer so current_tick resumes updating from where it was suspended
void resume_timer()
{
  //puts2(" -- resume -- ");
  disable();
  //getvect(USER_TIMER_INT);
  //setvect(USER_TIMER_INT, run_timer_isr);
  // timer_speed = 0x0400;
  set_timer_speed(timer_speed);
  enable();
  timer_not_installed_blocking = false;
}

// speeds up timer so current_tick updates faster
void speed_up_timer()
{
  //puts2(" -- speedup -- ");
  disable();
  timer_speed = timer_speed * 2;
  set_timer_speed(timer_speed);
  enable();
}

// slows timer so current_tick updating more slowly
void slow_down_timer()
{
  //puts2(" -- slowdown -- ");
  disable();
  if (timer_speed == 0)
    timer_speed = -1;
  timer_speed = timer_speed / 2;
  set_timer_speed(timer_speed);
  enable();
}

extern void draw_tasks();

// delay() causes the computer to idle for the given number of ticks.
// It works by the fact that timer updates current_tick.
// The function could be re-written as a pre-empt routine, except this
// function couldn't be pre-empted because currently only one pre-empt
// function can be installed at a time.
// The pre-emptor should be reserved for use by the scheduler only.
void delay(ticks_t number_of_ticks, ticks_t deadline)
{
  // must not call this if my_timer handler isn't installed
  if (timer_not_installed_blocking == true)
  {
    puts2("error: cannot call delay unless timer is enabled\n");
    exit(130);
  }

  tick_t finish_at = current_tick_ + number_of_ticks;

  if (current_tick_ > finish_at)
  {
    puts2("error: overflow condition in delay\n");
    exit(131);
  }

  while (current_tick_ < finish_at)
  {
    // wait for next tick
    tick_t next_tick = current_tick_ + 1;
    while (current_tick_ < next_tick)
      /* do nothing */ ;

    // update the bar view of the tasks every tick
    draw_tasks();
  }
}

tick_t current_tick()
{
  return current_tick_;
}

void set_current_tick(tick_t tick)
{
  current_tick_ = tick;
}
  
static
timer_driver_t baremetal_timer =
{
  "baremetal_timer",
  install_timer_isr,
  uninstall_timer_isr,
  enable_timer,
  disable_timer,
  suspend_timer,
  resume_timer,
  speed_up_timer,
  slow_down_timer
};

timer_driver_t& get_timer_ref() { return baremetal_timer; }

void init_timer_driver()
{
  // initialize variables
  current_tick_ = 0;   // number of ticks since timer was enabled
  timer_speed = 0x1000;
  timer_not_installed_blocking = true;
  preempt_at_tick = 0;
  user_preempt_routine = nullptr;
  user_preemptor_data = nullptr;
  block_preemptor = false;
  installed_timer_interrupt_in_service = false;

//  outportb(0x21,0xfe); // master - mask all but timer

  //outportb(PIC_MASTER_DATA, 0xfe); // master - mask all but timer
  //outportb(PIC_SLAVE_DATA, 0xff); // slave  - mask all

//  enable(); // asm("sti");

  //puts2("");
  //puts2("init_timer_driver");
  //timer = baremetal_timer;
}

void start_timer()
{
  enable(); // asm("sti");
}

void initialize_drivers()
{
}
