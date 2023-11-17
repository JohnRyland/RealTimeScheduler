/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

// Should look in to TSC_DEADLINE as another x86 timer method.
// Using TSC_DEADLINE would have much greater resolution if that is required
// but with some caveats.

#ifdef ENABLE_TIMER_INTEL_8253

#include "module/timer.h"
#include "module_manager.h"

void register_timer_intel_8253_module()
{
}

#endif // ENABLE_TIMER_INTEL_8253
