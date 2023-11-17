/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

//#include "conio.h"
#include "timer.h"
//#include "runtime.h"
#include "arch/x86/intrinsics.h"
#include "arch/x86/constants.h"
#include "kernel/exception_handler.h"
//#include "kernel/debug_logger.h"
#include "kernel/module_manager.h"
#include "module/serial.h"
#include "types/modules.h"


/*
struct test2_class_t
{
  test2_class_t()
  {
    k_log_early(SUCCESS, "_2_init!!");
  }
};

test2_class_t foobar;
*/


// Variables
static volatile tick_t current_tick_;   // number of ticks since timer was enabled
static volatile unsigned int timer_speed = 0x0000;
static bool timer_not_installed_blocking = true;
static tick_t preempt_at_tick = 0;
static preemptor_t user_preempt_routine = nullptr;
static void* user_preemptor_data = nullptr;
static bool block_preemptor = false;
static bool installed_timer_interrupt_in_service = false;

static
void toggle_char(int x)
{
  ((char*)VGA_TEXT_BASE)[x*2] = ((char*)VGA_TEXT_BASE)[x*2] == '^'  ? 'v'  : '^';
  ((char*)VGA_TEXT_BASE)[x*2+1] = ((char*)VGA_TEXT_BASE)[x*2+1] == 0x0B ? 0xB0 : 0x0B;
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
void timer_handler()
{
  if (installed_timer_interrupt_in_service == true)
    return;
  installed_timer_interrupt_in_service = true;
  current_tick_ = current_tick_ + 1;
  preemptor();
  installed_timer_interrupt_in_service = false;
}

//static
void keyboard_handler()
{
  // TODO: make interrupt driven
  toggle_char(0);
}

const uint16_t PS2_CMD_PORT_ADDRESS      = 0x64;
const uint16_t PS2_DATA_PORT_ADDRESS     = 0x60;
const uint8_t  PS2_GET_STATUS_BYTE       = 0x20;
const uint8_t  PS2_SET_STATUS_BYTE       = 0x60;
const uint8_t  PS2_SetDefaults           = 0xF6;
const uint8_t  PS2_EnablePacketStreaming = 0xF4;

void wait_for_read()
{
  for (int i = 0; i < 100000; i++)
    if ((inportb(PS2_CMD_PORT_ADDRESS) & 0x1) == 0x1)
      return;
//  Err("wait for mouse read timeout");
}

void wait_for_write()
{
  for (int i = 0; i < 100000; i++)
    if ((inportb(PS2_CMD_PORT_ADDRESS) & 0x2) == 0x0)
      return;
//  Err("wait for mouse write timeout");
}

uint8_t read_data_port()
{
  wait_for_read();
  return inportb(PS2_DATA_PORT_ADDRESS);
}

void write_command_port(uint8_t value)
{
  wait_for_write();
  outportb(PS2_CMD_PORT_ADDRESS, value);
}

void write_data_port(uint8_t value)
{
  wait_for_write();
  outportb(PS2_DATA_PORT_ADDRESS, value);
}

void send_command(uint8_t command)
{
  write_command_port(0xD4);
  write_data_port(command);
  if (read_data_port() != 0xFA)
    {}
    //Err("mouse did not respond to the command");
}

int16_t sign_extend(uint8_t packet)
{
  return (int16_t)(((uint16_t)packet) | 0xFF00);
}

// enum class MouseFlags : uint8_t
enum MouseFlags
{
  NONE           = 0,
  LEFT_BUTTON    = 1 << 0, /// Whether or not the left mouse button is pressed.
  RIGHT_BUTTON   = 1 << 1, /// Whether or not the right mouse button is pressed.
  MIDDLE_BUTTON  = 1 << 2, /// Whether or not the middle mouse button is pressed.
  ALWAYS_ONE     = 1 << 3, /// Whether or not the packet is valid or not.
  X_SIGN         = 1 << 4, /// Whether or not the x delta is negative.
  Y_SIGN         = 1 << 5, /// Whether or not the y delta is negative.
  X_OVERFLOW     = 1 << 6, /// Whether or not the x delta overflowed.
  Y_OVERFLOW     = 1 << 7, /// Whether or not the y delta overflowed.
};

/// A snapshot of the mouse flags, x delta and y delta.
struct MouseState
{
  MouseFlags  flags;
  int16_t     x, y;
};

uint8_t     current_packet;
MouseState  current_state;
MouseState  mouse_state;
size_t      prevIdx;
char        prevA, prevC;

// Initializes PS2 mouse on IRQ 12
void initialize_mouse()
{
  write_command_port(PS2_GET_STATUS_BYTE);
  auto status = read_data_port() | 0x02;
  write_command_port(PS2_SET_STATUS_BYTE);
  write_data_port(status & 0xDF);
  send_command(PS2_SetDefaults);
  send_command(PS2_EnablePacketStreaming);
  current_packet = 0;
  current_state.flags = MouseFlags::NONE;
  current_state.x = 0;
  current_state.y = 0;
  mouse_state.flags = MouseFlags::NONE;
  mouse_state.x = 40;
  mouse_state.y = 25;
  prevIdx = 0;
  prevA = 0;
  prevC = 0;
}

/*
inline bool operator&(MouseFlags a, MouseFlags b)
{
 return (static_cast<int>(a) & static_cast<int>(b));
}
*/

void process_x_movement(uint8_t packet)
{
  if (!(current_state.flags & MouseFlags::X_OVERFLOW))
    current_state.x = (current_state.flags & MouseFlags::X_SIGN) ? sign_extend(packet) : (int16_t)packet;
}

void process_y_movement(uint8_t packet)
{
  if (!(current_state.flags & MouseFlags::Y_OVERFLOW))
    current_state.y = (current_state.flags & MouseFlags::Y_SIGN) ? sign_extend(packet) : (int16_t)packet;
}

extern
void drawGraphicMousePointer(int x, int y);

void drawMousePointer(int x, int y)
{
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  drawGraphicMousePointer(x, y);

  if (x >= 80)
    x = 79;
  if (y >= 50)
    y = 49;

  size_t idx = y*160 + x*2;

  // Restore prev character
//  ((char*)VGA_TEXT_BASE)[prevIdx] = prevC;
  ((char*)VGA_TEXT_BASE)[prevIdx+1] = prevA;

  // Save old
  prevIdx = idx;
//  prevC = ((char*)VGA_TEXT_BASE)[idx];
  prevA = ((char*)VGA_TEXT_BASE)[idx+1];

  // Put new
//  ((char*)VGA_TEXT_BASE)[idx] = ((char*)VGA_TEXT_BASE)[idx] == '^'  ? 'v'  : '^';
//  ((char*)VGA_TEXT_BASE)[idx+1] = ((char*)VGA_TEXT_BASE)[idx+1] == 0x0B ? 0xB0 : 0x0B;
  ((char*)VGA_TEXT_BASE)[idx+1] = 0xB0;
}

void on_complete(MouseState state)
{
  mouse_state.x += state.x;
  mouse_state.y -= state.y;
  int x = mouse_state.x / 4;
  int y = mouse_state.y / 4;

  drawMousePointer(x, y);
}

/// Attempts to process a packet.
void process_packet(uint8_t packet)
{
  switch (current_packet)
  {
    case 0:
      if (!((MouseFlags)packet & MouseFlags::ALWAYS_ONE))
        return;
      current_state.flags = (MouseFlags)packet;
      break;
    case 1:
      process_x_movement(packet);
      break;
    case 2:
      process_y_movement(packet);
      on_complete(current_state);
      break;
    default:
      break;
  }
  current_packet++;
  if (current_packet == 3) // = (current_packet + 1) % 3;
    current_packet = 0;
}

static
void mouse_handler()
{
  //toggle_char(1);
  //read_data_port();
  process_packet(read_data_port());
}

static
void serial_port_handler()
{
  for (const module_t* serial = find_module_by_class(module_class::SERIAL_DRIVER); serial; serial = serial->next)
    ((serial_driver_vtable_t*)(serial->vtable))->irq_service_handler((serial_driver_t*)serial->instance);
}

extern "C"
void interrupt_handler(uint32_t interruptNumberOrMask)
{
  if (interruptNumberOrMask < 0x20)
    k_fault_handler((fault_t)interruptNumberOrMask);

//  if (interruptNumberOrMask >= 0x20 && interruptNumberOrMask < 0x28)
  if (interruptNumberOrMask == 0x27)
  {
    outportb(0x20, 0x0b);
//    if (!inportb(0x20))  // get the in-service register
    if ((inportb(0x20) & 0x80) == 0)  // get the in-service register
    {
      toggle_char(2);
    //  puts2(" **SPURIOUS IRQ7** ");
      return;  // It was a spurious interrupt, ignore
    }  
  }

//  if (interruptNumberOrMask >= 0x28 && interruptNumberOrMask < 0x30)
  if (interruptNumberOrMask == 0x2F)
  {
    outportb(0xA0, 0x0b);
    // if (!inportb(0xA0))  // get the in-service register
    if ((inportb(0xA0) & 0x80) == 0)  // get the in-service register
    {
      toggle_char(3);
      outportb(0x20, 0x20); // Send EOI (End-of-interrupt) to master PIC
  //    puts2(" **SPURIOUS IRQ15** ");
      return;  // It was a spurious interrupt, ignore
    }  
  }

  switch (interruptNumberOrMask - 0x20)
  {
    // TODO: map these with a configurable array that drivers can install in to
    case 0:  timer_handler();       break; // intel 8253 PIT 
    case 1:  keyboard_handler();    break; // intel 8048 Keyboard
    case 2:                         break; // intel 8259 slave PIC chained on IRQ2
    case 3:  serial_port_handler(); break; // serial 16650 COM2
    case 4:  serial_port_handler(); break; // serial 16650 COM1
    case 12: mouse_handler();       break; // PS2 mouse
    default:
       //mouse_handler(); break;
      toggle_char(4);
//      puts2(" **OTHER** ");
  }

//  if (interruptNumberOrMask >= 0x28 && interruptNumberOrMask <= 0x2F)

  if (interruptNumberOrMask >= 0x20 && interruptNumberOrMask < 0x30)
    outportb(0x20, 0x20); // Send EOI (End-of-interrupt) to master PIC

  if (interruptNumberOrMask >= 0x28 && interruptNumberOrMask < 0x30)
    outportb(0xA0, 0x20); // Send EOI (End-of-interrupt) to slave PIC
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
void delay(ticks_t number_of_ticks, ticks_t /*deadline*/)
{
  // must not call this if my_timer handler isn't installed
  if (timer_not_installed_blocking == true)
    k_critical_error(130, "error: cannot call delay unless timer is enabled\n");

  tick_t finish_at = current_tick_ + number_of_ticks;

  if (current_tick_ > finish_at)
    k_critical_error(133, "error: overflow condition in delay\n");

  draw_tasks();
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

void initialize_timer_driver()
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
  //puts2("initialize_timer_driver");
  //timer = baremetal_timer;
}

void start_timer()
{
//  gotoxy(0,0);puts2("HereX!");

  enable(); // asm("sti");

//  gotoxy(0,0);puts2("HereY!");
}

