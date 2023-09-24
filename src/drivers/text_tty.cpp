/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <termios.h>
// #include "conio.h"

#include "../../include/driver/text.h"


/*
static
unsigned random(unsigned x)
{
  return random() % x;
}
*/



static
bool load()
{
  return true;
}

static
bool unload()
{
  return true;
}

static
uint32_t width()
{
  // TODO: query
  return 80;
}

static
uint32_t height()
{
  // TODO: query
  return 50;
}

static
void gotoxy(uint32_t x, uint32_t y)
{
  printf("\033[%i;%iH", y, x);
}

static
void put_char(uint8_t /*attrib*/, char ch1)
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
void put_string(uint8_t attrib, const char* str)
{
  while (*str)
  {
    put_char(attrib, *str);
    ++str;
  }
  printf("\n");
}

static
void clear()
{
  printf("\033c");
}

static
text_vtable_t tty_vtable =
{
  .width      = width,
  .height     = height,
  .gotoxy     = gotoxy,
  .put_char   = put_char,
  .put_string = put_string,
  .clear      = clear
};

driver_t tty_driver =
{
  .type       = driver_class::TEXT_DISPLAY,
  .id         = 0x12023, // TODO: how to assign these? in the register?
  .name       = { "tty" },
  .next       = nullptr,
  .prev       = nullptr,
  //.details    = nullptr,
  .load       = load,
  .unload     = unload,
  .vtable     = &tty_vtable
};

void register_tty_driver()
{
  register_driver(tty_driver);
}
