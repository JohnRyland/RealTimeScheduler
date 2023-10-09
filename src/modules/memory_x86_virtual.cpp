/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_MEMORY_X86_VIRTUAL

#include "module/memory.h"
#include "module_manager.h"

static
void initialize()
{
}

static
void flush_page_tables()
{
}

static
page_set_t allocate_pages(int count, int flags)
{
}

static
void deallocate_pages(page_set_t pages)
{
}

static
void lock_pages(page_set_t pages)
{
}

static
memory_t map_pages(page_set_t pages)
{
}

static
void unmap_pages(page_set_t pages)
{
}

static
void unmap_memory(memory_t memory)
{
}

memory_model_vtable_t memory_x86_virtual_vtable =
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
module_t memory_x86_virtual_model = 
{
  .type    = module_class::MEMORY_MODEL,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "memory_x86_virt" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &memory_x86_virtual_vtable,
};

void register_memory_x86_virtual_model()
{
  module_register(memory_x86_virtual_model);
}

#endif // ENABLE_MEMORY_X86_VIRTUAL
