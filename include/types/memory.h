/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

struct page_t
{
  void*       physical_address;
};

struct page_set_t
{
  int         count;
  page_t*     pages;
};

struct memory_t
{
  page_set_t  page_set;
  void*       virtual_address;
};

// enum protect_t

