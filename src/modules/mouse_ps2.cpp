/*
  PS/2 Mouse driver
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "debug_logger.h"
#include "exception_handler.h"
//#include <stdint.h>

void Err(const char* msg)
{
  k_log_fmt(ERROR, "Error: %s\n", msg);
  exit(-1);
}

void EOI()
{
}

template <uint16_t address>
struct BytePort
{
  uint8_t read()
  {
    // inb
    return 0;
  }
  void write(uint8_t /*val*/)
  {
    // outb
  }
};


// interface to a ps2 mouse

const uint16_t ADDRESS_PORT_ADDRESS = 0x64;
const uint16_t DATA_PORT_ADDRESS    = 0x60;
const uint8_t  GET_STATUS_BYTE      = 0x20;
const uint8_t  SET_STATUS_BYTE      = 0x60;

/// Represents the flags currently set for the mouse.
enum class MouseFlags : uint8_t
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

inline MouseFlags operator|(MouseFlags a, MouseFlags b)
{
    return static_cast<MouseFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(MouseFlags a, MouseFlags b)
{
    return (static_cast<int>(a) & static_cast<int>(b));
}

enum class Command : uint8_t
{
  EnablePacketStreaming = 0xF4,
  SetDefaults           = 0xF6,
};

/// A snapshot of the mouse flags, x delta and y delta.
struct MouseState
{
  bool left_button_down() const  { return  (flags & MouseFlags::LEFT_BUTTON); }
  bool left_button_up() const    { return !(flags & MouseFlags::LEFT_BUTTON); }
  bool right_button_down() const { return  (flags & MouseFlags::RIGHT_BUTTON); }
  bool right_button_up() const   { return !(flags & MouseFlags::RIGHT_BUTTON); }
  bool x_moved() const           { return x != 0; }
  bool y_moved() const           { return y != 0; }
  bool moved() const             { return x_moved() || y_moved(); }
  int16_t get_x() const          { return x; }
  int16_t get_y() const          { return y; }

  MouseFlags  flags = MouseFlags::NONE;
  int16_t     x = 0, y = 0;
};

typedef void (*MouseCallback)(MouseState);  //  Option<fn(MouseState)>

// A basic interface to interact with a PS2 mouse.
struct Mouse
{
  BytePort<ADDRESS_PORT_ADDRESS> command_port;
  BytePort<DATA_PORT_ADDRESS>    data_port;
  uint8_t                        current_packet = 0;
  MouseState                     current_state;
  MouseState                     completed_state;
  MouseCallback                  on_complete = nullptr;

  /// Returns the last completed state of the mouse.
  MouseState get_state() const
  {
    return completed_state;
  }

  /// Attempts to initialize a `Mouse`. If successful, interrupts will be generated
  /// as `PIC offset + 12`.
  void init()
  {
    write_command_port(GET_STATUS_BYTE);
    auto status = read_data_port() | 0x02;
    write_command_port(SET_STATUS_BYTE);
    write_data_port(status & 0xDF);
    send_command(Command::SetDefaults);
    send_command(Command::EnablePacketStreaming);
  }

  void handle_interrupt()
  {
    uint8_t packet = read_data_port();
    process_packet(packet);
  }

  /// Attempts to process a packet.
  void process_packet(uint8_t packet)
  {
    switch (current_packet)
    {
      case 0:
        {
          auto flags = (MouseFlags)packet;
          if (!(flags & MouseFlags::ALWAYS_ONE))
            return;
          current_state.flags = flags;
        }
        break;
      case 1:
        process_x_movement(packet);
        break;
      case 2:
        process_y_movement(packet);
        completed_state = current_state;
        if (on_complete)
          on_complete(completed_state);
        break;
      default:
        break;
    }
    current_packet = (current_packet + 1) % 3;
  }

  /// Sets the `on_complete` function to be called when a packet is completed.
  void set_on_complete(MouseCallback handler)
  {
    on_complete = handler;
  }

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

  void send_command(Command command)
  {
    write_command_port(0xD4);
    write_data_port((uint8_t)command);
    if (read_data_port() != 0xFA)
      Err("mouse did not respond to the command");
  }

  int16_t sign_extend(uint8_t packet)
  {
    return (int16_t)(((uint16_t)packet) | 0xFF00);
  }

  uint8_t read_data_port()
  {
    wait_for_read();
    return data_port.read();
  }

  void write_command_port(uint8_t value)
  {
    wait_for_write();
    command_port.write(value);
  }

  void write_data_port(uint8_t value)
  {
    wait_for_write();
    data_port.write(value);
  }

  void wait_for_read()
  {
    for (int i = 0; i < 100000; i++)
      if ((command_port.read() & 0x1) == 0x1)
        return;
    Err("wait for mouse read timeout");
  }

  void wait_for_write()
  {
    for (int i = 0; i < 100000; i++)
      if ((command_port.read() & 0x2) == 0x0)
        return;
    Err("wait for mouse write timeout");
  }
};

