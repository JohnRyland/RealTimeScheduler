/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_SCHEDULER_REALTIME

#include "module/scheduler.h"
#include "module_manager.h"

static
bool initialize()
{
  return true;
}

static
task_acceptance_t add_task(task_t* /*task*/)
{
  return task_acceptance_t::schedule_full;
}

scheduler_vtable_t scheduler_realtime_vtable =
{
  .initialize = initialize,
  .add_task = add_task,
};

static
module_t scheduler_realtime_module = 
{
  .type    = module_class::SCHEDULER_MODULE,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "sched_realtime" },
  .next    = nullptr,
  .prev    = nullptr,
  .vtable  = &scheduler_realtime_vtable,
};

void register_scheduler_realtime_module()
{
  module_register(scheduler_realtime_module);
}

#endif // ENABLE_SCHEDULER_REALTIME
