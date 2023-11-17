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
#include "kernel/bios.h"
#include "arch/x86/constants.h"

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

  // Change back to text mode
  call_real_mode_interrupt(0x10, 0x3);
  call_real_mode_interrupt(0x10, 0x1112);
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
  disp->cur_x = (x > 0) ? x - 1 : 0;
  disp->cur_y = (y > 0) ? y - 1 : 0;
}

extern void drawText(uint8_t attrib, int x, int y, const char* str);

static
void scroll_up()
{
  mem_cpy((void*)VGA_TEXT_BASE, (void*)((uint16_t*)VGA_TEXT_BASE + 80), 160*49);
  mem_set((void*)((uint16_t*)VGA_TEXT_BASE + 80*49), 0, 160);
  // TODO: scroll-up for the graphics mode
}

static
void advance_xy(text_display_t* disp, int x, bool y)
{
  disp->cur_x += x;
  if (y || disp->cur_x >= 80)
    disp->cur_x = 0, disp->cur_y++;
  if (disp->cur_y >= 50)
  {
    disp->cur_y = 49;
    scroll_up();
  }
}

static
void put_char(text_display_t* disp, uint8_t attrib, char ch)
{
  if (disp->mode == 0x01)
  {
    if (ch != '\n')
    {
      uint16_t buf = *((uint8_t*)&ch) & 0xFF;
      drawText(attrib, disp->cur_x * 8, disp->cur_y * 16, (char*)&buf);
    }
  }

  if (ch != '\n')
    ((short*)VGA_TEXT_BASE)[disp->cur_y*80 + disp->cur_x] = (attrib << 8) | (unsigned)((unsigned char)ch);

  advance_xy(disp, 1, ch == '\n');
}

static
void put_string(text_display_t* disp, uint8_t attrib, const char* str)
{
  while (*str)
    put_char(disp, attrib, *str++);
  put_char(disp, attrib, '\n');
}

static
void clear(text_display_t* disp)
{
  mem_set((void*)VGA_TEXT_BASE, 0, 80*50*2);
  disp->cur_x = 0;
  disp->cur_y = 0;
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

void pre_initialize_text()
{
  text_vga_instance.mode = 0;
  text_vga_instance.cur_x = 0;
  text_vga_instance.cur_y = 0;
  text_vga_instance.width = 80;
  text_vga_instance.height = 50;
}

void post_initialize_text()
{
  text_vga_instance.mode = 1;
  text_vga_instance.cur_x = 0;
  text_vga_instance.cur_y = 0;
  text_vga_instance.width = 80;
  text_vga_instance.height = 50;
}

extern
void drawMousePointer2(int x, int y);

void drawGraphicMousePointer(int x, int y)
{
  if (text_vga_instance.mode == 1)
  {
    drawMousePointer2(x, y);
  }
}


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
