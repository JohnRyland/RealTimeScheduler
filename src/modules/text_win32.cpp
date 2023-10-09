/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_TEXT_WIN32

#include "module/text.h"
#include "module_manager.h"

#include <windows.h>

static
void initialize(text_display_t*)
{
}

static
uint32_t width(text_display_t*)
{
}

static
uint32_t height(text_display_t*)
{
}

static
void goto_xy(text_display_t*, uint32_t x, uint32_t y)
{
}

static
void put_char(text_display_t*, uint8_t /*attrib*/, char ch1)
{
}

static
void put_string(text_display_t* disp, uint8_t attrib, const char* str)
{
  while (*str)
  {
    put_char(disp, attrib, *str);
    ++str;
  }
  put_char(disp, attrib, '\n');
}

static
void clear(text_display_t*)
{
}

static
text_display_vtable_t text_win32_vtable =
{
  .initialize = initialize,
  .width      = width,
  .height     = height,
  .goto_xy    = goto_xy,
  .put_char   = put_char,
  .put_string = put_string,
  .clear      = clear
};

module_t text_win32_module =
{
  .type       = module_class::TEXT_DISPLAY,
  .id         = 0x12023, // TODO: how to assign these? in the register?
  .name       = { "text_win32" },
  .next       = nullptr,
  .prev       = nullptr,
  .vtable     = &text_win32_vtable,
  .instance   = nullptr,
};

void register_text_win32_display()
{
  module_register(text_win32_module);
}

#endif // ENABLE_TEXT_WIN32
