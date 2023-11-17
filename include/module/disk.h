/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

struct disk_parameters_t
{
  size_t  sector_size;
  size_t  sector_count;
  // other ??
};

struct sector_desc_t
{
  size_t  sector_id;
  void*   sector_ptr;
};

struct sector_set_t
{
  sector_desc_t*  sectors;
};

// this is raw disk I/O API. higher level is volume management and then on
// top of that is file-system types and finally file access, which are other APIs.
struct disk_vtable_t
{
  void (*initialize)();
  void (*flush)();
  disk_parameters_t (*get_parameters)();
  sector_set_t (*allocate_sector_set)(size_t sector_count);
  void (*deallocate_sector_set)(sector_set_t sectors);
  void (*read_sectors)(sector_set_t sectors);
  void (*write_sectors)(sector_set_t sectors);
};
