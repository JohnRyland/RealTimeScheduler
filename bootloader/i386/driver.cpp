/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "timer.h"
#include "runtime.h"
#include "constants.h"


#define USER_TIMER_INT 0x08  // Could use INT 0x1C or INT 0x70 instead with a few modifications


#define interrupt
#define NULL nullptr
typedef void (*isr_routine_t)(...);


// Variables
static volatile tick_t current_tick_;   // number of ticks since timer was enabled
static void interrupt (*old_timer)(...);
static volatile unsigned int timer_speed = 0x0000;
static bool timer_not_installed_blocking = true;
static tick_t preempt_at_tick = 0;
static isr_routine_t user_preempt_routine = NULL;
static bool block_preemptor = false;
static bool installed_timer_interrupt_in_service = false;



int inportb(int port)
{
  int val = 0;
  __asm__ __volatile__ (
    "movl %%eax,%%eax\n"
    "movl %1,%%edx\n"
    "inb  %%dx,%%al  # read the port\n"
    "movl %%eax,%0"
    : "=r"(val)
    : "r"(port)
    : "%eax", "%edx");
  return val;
}

void outportb(int port, int val)
{
  __asm__ __volatile__ (
    "xorl %0,%%eax\n"
    "movl %1,%%edx\n"
    "outb %%al,%%dx  # write to the port\n"
    : : "r"(val), "r"(port)
    : "%eax", "%edx");
}

void enable()
{
  __asm__ __volatile__ ("sti");
}

void disable()
{
  __asm__ __volatile__ ("cli");
}

isr_routine_t getvect(int vec)
{
  return nullptr;
}

void setvect(int vec, isr_routine_t)
{
}



// Current text position
static int curX;
static int curY;

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
  return inportb(PPI_DATA); // read the key that is pressed
}

void putch(char ch)
{
  curX++;
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









void init_timer_driver()
{
  // TODO
}

/*
void delay(ticks_t number_of_ticks, tick_t deadline)
{
  // TODO
}

tick_t current_tick()
{
  // TODO
  return 0;
}

void set_current_tick(tick_t tick)
{
  // TODO
}
*/





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
	 (user_preempt_routine == NULL) || (block_preemptor == true))
    return;

  if (current_tick_ >= preempt_at_tick)
  {
    block_preemptor = true;
    // run the user function
    user_preempt_routine();
    // uninstall it when done
    preempt_at_tick = 0;
    user_preempt_routine = NULL;
    block_preemptor = false;
  }
}

static
void interrupt run_timer_isr(...)
{
  if (installed_timer_interrupt_in_service == true)
    return;
  installed_timer_interrupt_in_service = true;
  current_tick_++;
  preemptor();
  old_timer();
  installed_timer_interrupt_in_service = false;
}


// Implementation

// installs a user function that will get called at the given tick
bool install_timer_isr(tick_t event_time, void (*user_func)(...))
{
  // if it's unsafe to install a user_func or one is already installed
  // then return false
  if ((user_preempt_routine != NULL) || (block_preemptor == true))
    return false;
  block_preemptor = true;
  preempt_at_tick = event_time;
  user_preempt_routine = user_func;
  block_preemptor = false;
  return true;
}

// uninstalls an installed user function
bool uninstall_timer_isr()
{
  // if it's unsafe to uninstall a user_func then return false
  if (block_preemptor == true)
    return false;
  block_preemptor = true;
  preempt_at_tick = 0;
  user_preempt_routine = NULL;
  block_preemptor = false;
  return true;
}

// starts timer so that current_tick will automatically update
void enable_timer()
{
  disable();
  old_timer = getvect(USER_TIMER_INT);
  setvect(USER_TIMER_INT, run_timer_isr);
  timer_speed = 0x0400;
  set_timer_speed(timer_speed);    // approx 1000Hz
  current_tick_ = 0;
  enable();
  timer_not_installed_blocking = false;
}

// stops timer
void disable_timer()
{
  timer_not_installed_blocking = true;
  disable();
  timer_speed = 0x0000;
  set_timer_speed(timer_speed);    // default 18.2Hz
  setvect(USER_TIMER_INT, old_timer);
  enable();
}

// suspends timer so current_tick can be temporarily made to stop updating
void suspend_timer()
{
  disable_timer();
}

// resumes timer so current_tick resumes updating from where it was suspended
void resume_timer()
{
  disable();
  old_timer = getvect(USER_TIMER_INT);
  setvect(USER_TIMER_INT, run_timer_isr);
  timer_speed = 0x0400;
  set_timer_speed(timer_speed);
  enable();
  timer_not_installed_blocking = false;
}

// speeds up timer so current_tick updates faster
void speed_up_timer()
{
  disable();
  timer_speed = timer_speed * 2;
  set_timer_speed(timer_speed);
  enable();
}

// slows timer so current_tick updating more slowly
void slow_down_timer()
{
  disable();
  if (timer_speed == 0)
    timer_speed = -1;
  timer_speed = timer_speed / 2;
  set_timer_speed(timer_speed);
  enable();
}

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
    exit(0);
  }

  tick_t finish_at = current_tick_ + number_of_ticks;

  if (current_tick_ > finish_at)
  {
    puts2("error: overflow condition in delay\n");
    exit(0);
  }

  while (current_tick_ < finish_at)
  {
    // wait for next tick
    tick_t next_tick = current_tick_ + 1;
    while (current_tick_ < next_tick)
      /* do nothing */ ;

    
    // update the bar view of the tasks every tick
    //draw_tasks();

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

/*
struct timer_driver_t
{
  const char* name;
  bool (*install_preemptor)(tick_t event_time, preemptor_t user_func, void* user_data);
  bool (*uninstall_preemptor)();
  void (*enable)();
  void (*disable)();
  void (*suspend)();
  void (*resume)();
  void (*speed_up)();
  void (*slow_down)();
  //void (*set_speed)(unsigned int rate);
};
*/

timer_driver_t timer;

