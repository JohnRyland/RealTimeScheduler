/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_MEMORY_GENERIC

#include "module/memory.h"
#include "module_manager.h"

#include <cstdlib>

#define PAGE_SIZE    4096

static
void initialize()
{
}

static
void flush_page_tables()
{
}

static
page_set_t allocate_pages(int count, int /*flags*/)
{
  page_set_t set;
  set.count = count;
  set.pages = (page_t*)malloc(sizeof(page_t) * count);
  uint8_t* mem = (uint8_t*)malloc(PAGE_SIZE * count);
  for (int i = 0; i < count; ++i)
    set.pages[i].physical_address = (void*)&mem[i * PAGE_SIZE];
  return set;
}

static
void deallocate_pages(page_set_t pages)
{
  free(pages.pages[0].physical_address);
}

static
void lock_pages(page_set_t /*pages*/)
{
}

static
memory_t map_pages(page_set_t pages)
{
  memory_t mem;
  mem.page_set = pages;
  mem.virtual_address = pages.pages[0].physical_address;
  return mem;
}

static
void unmap_pages(page_set_t /*pages*/)
{
}

static
void unmap_memory(memory_t /*memory*/)
{
}

memory_model_vtable_t memory_generic_vtable =
{
  .initialize = initialize,
  .flush_page_tables = flush_page_tables,
  .allocate_pages = allocate_pages,
  .deallocate_pages = deallocate_pages,
  .lock_pages = lock_pages,
  .map_pages = map_pages,
  .unmap_pages = unmap_pages,
  .unmap_memory = unmap_memory,
};

static
module_t memory_generic_model = 
{
  .type    = module_class::MEMORY_MODEL,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "memory_generic" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &memory_generic_vtable,
};

void register_memory_generic_model()
{
  module_register(memory_generic_model);
}

#endif // ENABLE_MEMORY_GENERIC
