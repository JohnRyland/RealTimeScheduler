/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

// flags:
//   contiguous (for DMA)
//   read/write/execute
// C++ has enum type, but no flags type that is a bit which
// is set and can be or'ed and and'd. Some kind of class for flags?

struct memory_model_vtable_t
{
  void (*initialize)();
  void (*flush_page_tables)();
  page_set_t (*allocate_pages)(int count, int flags);  // physical
  void (*deallocate_pages)(page_set_t pages);  // physical
  void (*lock_pages)(page_set_t pages);       // pin the mapping, is this needed?
  memory_t (*map_pages)(page_set_t pages);    // virtual
  void (*unmap_pages)(page_set_t pages);      // virtual
  void (*unmap_memory)(memory_t memory);      // virtual
};

/*
 possible types needed:
   page_t
   prot_t
   stats_t
   address_space_t
  funcs:
    address_space_t create_address_space();
    void destroy_address_space(address_space_t address_space);
  user space heap manager needs something like:
    int sbrk(int top_of_heap = 0);
*/
