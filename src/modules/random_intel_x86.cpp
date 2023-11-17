/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_RANDOM_INTEL_X86

#include "module/random.h"
#include "module_manager.h"

// https://en.wikipedia.org/wiki/RDRAND
// Note: FreeBSD stopped using RDRAND over issues of trust. There might be more secure ways,
// but just for general purpose random numbers, this is good enough, just don't use for any
// thing critical without checking out the caveats.

static
void initialize()
{
  asm volatile ( "rdseed %eax" );
}

static
int random(int min, int max)
{
  // assert max > min => how to handle this? - probably just a max, and let the caller code adjust for the min value

  int rand;

  //  qemu isn't able to emulate this instruction
  //asm volatile ( "rdrandl %%eax" : "=A"(rand) );         // This is very slow
  //  we use some crappy replacement code just for now
  asm volatile ( "xorl %%edx,%%eax" : "=A"(rand) );

  // TODO: this hasn't been unbiased yet - the wrap around will wrap only partially
  // around the range so there will be the first set of numbers which are drawn 1 more
  // time on average than the second set.  Example, you have a generator for 0-255. But
  // you ask for 0-250. If you get every number from 0-255 exactly once, then you will
  // get 0 to 4 twice, and 5 to 250 once.  
  return min + (rand % (max-min));

  /*
    // https://stackoverflow.com/questions/10984974/why-do-people-say-there-is-modulo-bias-when-using-a-random-number-generator
    I'm thinking the solution might be something like this:
    unsigned range = max - min;
    while (range*2 > range)
      range = range*2;
    unsigned rand = -1;
    while (rand > range)
      asm volatile ( "rdrand" : "=A"(rand) );
    return min + (rand % (max-min));
  */
}

random_device_vtable_t random_vtable =
{
  .initialize = initialize,
  .random = random,
};

static
module_t random_device = 
{
  .type    = module_class::RANDOM_DEVICE,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "random_x86" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &random_vtable,
  .instance = nullptr,
};

void register_random_intel_x86_device()
{
  module_register(random_device);
}

#endif // ENABLE_RANDOM_INTEL_X86
