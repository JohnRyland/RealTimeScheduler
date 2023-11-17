/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_RANDOM_GENERIC

#include "module/random.h"
#include "module_manager.h"
#include <cstdlib>
#include <ctime>

static
void initialize()
{
  srand(clock());
}

static
int random(int min, int max)
{
  // TODO: this hasn't been unbiased yet
  return min + (rand() % (max-min));
}

random_device_vtable_t random_generic_vtable =
{
  .initialize = initialize,
  .random = random,
};

static
module_t random_generic_device = 
{
  .type    = module_class::RANDOM_DEVICE,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "random_generic" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &random_generic_vtable,
};

void register_random_generic_device()
{
  module_register(random_generic_device);
}

#endif // ENABLE_RANDOM_GENERIC
