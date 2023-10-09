/*
 x86 OS Kernel Debug Logging
 Copyright (C) 2023, John Ryland
 All rights reserved.
*/

#include "module/text.h"
#include "module_manager.h"
#include "debug_logger.h"
#include <cstdarg>

//static int col;
//static int row;
static char attrib;

//static
void goto_xy(unsigned x, unsigned y)
{
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  text_display_t*        text_display_data = (text_display_t*)text_display_module->instance;
  text_display_vtable_t* text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
  text_display_functions->goto_xy(text_display_data, x, y);
}

static
void log_char(char ch)
{
  if (!modules_initialized())
    return;
  module_t const* text_display_module = find_module_by_class(module_class::TEXT_DISPLAY);
  text_display_t*        text_display_data = (text_display_t*)text_display_module->instance;
  text_display_vtable_t* text_display_functions = (text_display_vtable_t*)text_display_module->vtable;
  text_display_functions->put_char(text_display_data, attrib, ch);
}

static
void log_str(const char* msg)
{
  while (*msg)
    log_char(*msg++);
}

template <unsigned BASE>
static inline
void log_number(unsigned val)
{
  static const uint8_t base = (BASE > 16) ? 16 : BASE;  // clamp it to 16 or less
  static const char digits[] = "0123456789ABCDEF";      // character map
  // char buf[22] = {}; // with intializiation like this, it breaks at runtime if build with x86_64-elf-gcc.
  char buf[22];                     // big enough for 64-bit number in decimal
  buf[21] = 0;                      // put nul at what will be the end of the string
  char* ptr = buf + 21;             // point to the destination in the string of the last character of the number
  do {                              // loop at least once, so if val is zero it will put at least one character which is '0'.
    *(--ptr) = digits[val % base];  // keep writing out the next number working to the left (to the most significant digits)
  } while (val /= base);            // until we have processed every decimal digit.
  log_str(ptr);                   // now print the resulting string from the most significant digit we last wrote.
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
        case '\\':
            fmt++;
            switch (*fmt)
            {
                case 'a': log_char('\a'); break;
                case 'b': log_char('\b'); break;
                case 'e': log_char('\e'); break;
                case 'f': log_char('\f'); break;
                case 'n': log_char('\n'); break;
                //case 'p': log_char('\p'); break;
                case 'r': log_char('\r'); break;
                case 't': log_char('\t'); break;
                //case 'u': log_char('\u'); break;
                case 'v': log_char('\v'); break;
                case 0: fmt--; break;
            }
            break;
        case '%':
            fmt++;
            switch (*fmt)
            {
                case 'd': log_number<10>(va_arg(ap, int)); break;
                case 'i': log_number<10>(va_arg(ap, int)); break;
                case 'x': log_number<16>(va_arg(ap, int)); break;
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

/*
static
void new_line()
{
  col = 0;
  row++;
  if (row > 49)
  {
    char* videoMemory = (char*)0xB8000;
    for (int i = 0; i < 160*49; ++i)
      videoMemory[i] = videoMemory[i+160];
    for (int i = 0; i < 160; ++i)
      videoMemory[i+160*49] = 0;
    row--;
  }
}

static
void add_char(char ch)
{
  char* videoMemory = (char*)0xB8000;
  videoMemory[row*160+col*2+0] = ch;
  videoMemory[row*160+col*2+1] = attrib;
  col++;
  if (col >= 80)
    new_line();
}

//static
void print(const char* string)
{
  while (*string)
  {
    if (*string == '\n')
      new_line();
    else
      add_char(*string);
    string++;
  }
  new_line();
}

//static
void printn(const char* string, int count)
{
  for (int i = 0; i < count; ++i)
    add_char(string[i]);
}

//static
void stamp_hex32(char* dest, uint32_t val)
{
  static const char hex_digit[] = "0123456789ABCDEF";
  for (int i = 0; i < 8; ++i)
    dest[i] = hex_digit[(val >> ((7 - i)*4)) & 0xF];
}
*/
