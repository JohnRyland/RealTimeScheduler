/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_TEXT_VGA

#include "module/text.h"
#include "module_manager.h"
#include "runtime/memory.h"

#include "constants.h"
#include "x86.h"

struct text_display_t
{
  int mode;
  int width;
  int height;
  int cur_x;
  int cur_y;
};

static
void initialize(text_display_t* disp)
{
  disp->mode = 0x03;
  disp->width = 80;
  disp->height = 50;
  disp->cur_x = 0;
  disp->cur_y = 0;
}

static
uint32_t width(text_display_t* disp)
{
  return disp->width;
}

static
uint32_t height(text_display_t* disp)
{
  return disp->height;
}

static
void goto_xy(text_display_t* disp, uint32_t x, uint32_t y)
{
  disp->cur_x = x;
  disp->cur_y = y;
}

static
void put_char(text_display_t* disp, uint8_t attrib, char ch)
{
  disp->cur_x++;
  disp->cur_y = disp->cur_y % 51;
  disp->cur_x = disp->cur_x % 81;
  if (ch == '\n')
    disp->cur_x = 0, disp->cur_y++;
  else
    ((short*)VGA_TEXT_BASE)[disp->cur_y*80 + disp->cur_x - 1] = (attrib << 8) | (unsigned)((unsigned char)ch);
}

static
void put_string(text_display_t* disp, uint8_t attrib, const char* str)
{
  while (*str)
    put_char(disp, attrib, *str++);
  put_char(disp, attrib, '\n');
}

static
void clear(text_display_t*)
{
  mem_set((void*)VGA_TEXT_BASE, 0, 80*50*2);
}

static
text_display_vtable_t text_vga_vtable =
{
  .initialize = initialize,
  .width      = width,
  .height     = height,
  .goto_xy    = goto_xy,
  .put_char   = put_char,
  .put_string = put_string,
  .clear      = clear
};

static
text_display_t text_vga_instance;

module_t text_vga_module =
{
  .type       = module_class::TEXT_DISPLAY,
  .id         = 0x12023, // TODO: how to assign these? in the register?
  .name       = { "text_vga" },
  .next       = nullptr,
  .prev       = nullptr,
  .vtable     = &text_vga_vtable,
  .instance   = &text_vga_instance,
};

void register_text_vga_display()
{
  module_register(text_vga_module);
}

#endif // ENABLE_TEXT_VGA
