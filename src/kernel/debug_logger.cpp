/*
 x86 OS Kernel Debug Logging
 Copyright (C) 2023, John Ryland
 All rights reserved.
*/

#include "module/text.h"
#include "module/serial.h"
#include "module_manager.h"
#include "debug_logger.h"
#include <stdarg.h>

static char attrib;

static bool both = true;

// static
void log_char(char ch1)
{
  if (!modules_initialized())
    return;

  if (both) // TODO: need some more logic here, we could have VT (virtual terminals) where this buffers to
  {
    module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
    text_display_t*        text_display_data = (text_display_t*)text_display_module->instance;
    text_display_vtable_t* text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
    text_display_functions->put_char(text_display_data, attrib, ch1);
  }

  // send to serial monitor
  const module_t* serial = find_module_by_class(module_class::SERIAL_DRIVER);
  if (serial) // TODO: We need some kind of kernel parameters/config to say which device is the debug serial device
  {
    unsigned char ch = static_cast<unsigned char>(ch1);
    // These are each 3-bytes in utf-8 (but were a single byte in old DOS code-page / ROM font)
    const char* charLUT = "│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌";
    if (ch >= 0xB3 && ch <= 0xDA)
    {
      // This translation logic to escape codes belongs in some kind of TTY driver code (tty logic in both directions, encode and decode, independant of the device)
      ch -= 0xB3;
      char c1 = charLUT[ch*3+0];
      char c2 = charLUT[ch*3+1];
      char c3 = charLUT[ch*3+2];
      ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, (uint8_t)c1);
      ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, (uint8_t)c2);
      ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, (uint8_t)c3);
    }
    else if (ch == 0xDB)
    {
      ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, (uint8_t)"▇"[0]);
    }
    else
    {
      ((serial_driver_vtable_t*)(serial->vtable))->send((serial_driver_t*)serial->instance, (uint8_t)ch);
    }
  }
}

void clear_screen()
{
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  text_display_t*        text_display_data = (text_display_t*)text_display_module->instance;
  text_display_vtable_t* text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
  text_display_functions->clear(text_display_data);

  // send control codes to serial monitor
  both = false;
  log_char('\033');
  log_char('c');
 // k_log_fmt(TRACE, "\\033[2J");
  both = true;
}

//static
void goto_xy(unsigned x, unsigned y)
{
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  text_display_t*        text_display_data = (text_display_t*)text_display_module->instance;
  text_display_vtable_t* text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
  text_display_functions->goto_xy(text_display_data, x, y);

  // send control codes to serial monitor
  both = false;
  k_log_fmt(TRACE, "\033[%i;%iH", y + 1, x + 1);
  both = true;
}

static
void log_str(const char* msg)
{
  while (*msg)
    log_char(*msg++);
}

template <typename T, unsigned BASE, int PAD=0>
static inline
void log_number(T val)
{
  static_assert(PAD < 22, "Pad value too large");
  static const uint8_t base = (BASE > 16) ? 16 : BASE;  // clamp it to 16 or less
  static const char digits[] = "0123456789ABCDEF";      // character map
  // char buf[22] = {}; // with intializiation like this, it breaks at runtime if build with x86_64-elf-gcc.
  char buf[22];                     // big enough for 64-bit number in decimal
  buf[21] = 0;                      // put nul at what will be the end of the string
  char* ptr = buf + 21;             // point to the destination in the string of the last character of the number
  int pad = PAD;
  do {                              // loop at least once, so if val is zero it will put at least one character which is '0'.
    *(--ptr) = digits[val % base];  // keep writing out the next number working to the left (to the most significant digits)
    --pad;
  } while (val /= base);            // until we have processed every decimal digit.
  while (pad-- > 0)
    *(--ptr) = '0';
  log_str(ptr);                     // now print the resulting string from the most significant digit we last wrote.
}

static
void log_float(double number)
{
    number = number / 2.0;
}

#define   DARK                0x08
#define   LIGHT               0x07
#define   BOLD                0x0F
#define   GREEN               0x0A
#define   YELLOW              0x0E
#define   RED                 0x04
#define   BRIGHT_RED          0x0C

void k_log_vfmt(log_level level, const char* fmt, va_list ap)
{
  switch (level)
  {
    case TRACE:    attrib = DARK;   break;
    case DEBUG:    attrib = LIGHT;  break;
    case NORMAL:   attrib = BOLD;   break;
    case SUCCESS:  attrib = GREEN;  break;
    case WARNING:  attrib = YELLOW; break;
    case ERROR:    attrib = RED;    break;
    case CRITICAL: attrib = BRIGHT_RED; break;
  }

  while (*fmt)
  {
    switch (*fmt)
    {
        case '%':
            fmt++;
            switch (*fmt)
            {
                // TODO: everything is treated as unsigned, also floats not supported yet
                case 'd': log_number<int, 10>(va_arg(ap, int)); break;
                case 'i': log_number<int, 10>(va_arg(ap, int)); break;
                case 'x': log_number<uint32_t, 16, 8>(va_arg(ap, uint32_t)); break;
                case 'X': log_number<uint64_t, 16, 16>(va_arg(ap, uint64_t)); break;
                case '%': log_char('%'); break;
                case 'f': log_float(va_arg(ap, double)); break;
                case 'c': log_char(va_arg(ap, int)); break;
                case 's': log_str(va_arg(ap, const char*)); break;
                case 0: fmt--; break;
            }
            break;
        default:
            log_char(*fmt);
            break;
    }
    fmt++;
  }
}

void k_log_fmt(log_level level, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  k_log_vfmt(level, fmt, ap);
  va_end(ap);
}
