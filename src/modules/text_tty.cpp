/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_TEXT_TTY

#include "module/text.h"
#include "module_manager.h"
#include <cstdio>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <termios.h>

static
void initialize(text_display_t*)
{
}

static
uint32_t width(text_display_t*)
{
  // TODO: query
  return 80;
}

static
uint32_t height(text_display_t*)
{
  // TODO: query
  return 50;
}

static
void goto_xy(text_display_t*, uint32_t x, uint32_t y)
{
  printf("\033[%i;%iH", y + 1, x + 1);
}

static
void put_char(text_display_t*, uint8_t /*attrib*/, char ch1)
{
  unsigned char ch = static_cast<unsigned char>(ch1);
  // These are each 3-bytes in utf-8 (but were a single byte in old DOS code-page / ROM font)
  const char* charLUT = "│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌";
  if (ch >= 0xB3 && ch <= 0xDA)
  {
    ch -= 0xB3;
    char c1 = charLUT[ch*3+0];
    char c2 = charLUT[ch*3+1];
    char c3 = charLUT[ch*3+2];
    printf("%c%c%c", c1, c2, c3);
  }
  else if (ch == 0xDB)
  {
    printf("▇");
  }
  else
  {
    printf("%c", ch);
  }
}

static
void put_string(text_display_t* disp, uint8_t attrib, const char* str)
{
  while (*str)
  {
    put_char(disp, attrib, *str);
    ++str;
  }
  //printf("\n");
}

static
void clear(text_display_t*)
{
  printf("\033c");
}

static
text_display_vtable_t text_tty_vtable =
{
  .initialize = initialize,
  .width      = width,
  .height     = height,
  .goto_xy    = goto_xy,
  .put_char   = put_char,
  .put_string = put_string,
  .clear      = clear
};

module_t text_tty_module =
{
  .type       = module_class::TEXT_DISPLAY,
  .id         = 0x12023, // TODO: how to assign these? in the register?
  .name       = { "text_tty" },
  .next       = nullptr,
  .prev       = nullptr,
  .vtable     = &text_tty_vtable,
  .instance   = nullptr,
};

void register_text_tty_display()
{
  module_register(text_tty_module);
}

#endif // ENABLE_TEXT_TTY

