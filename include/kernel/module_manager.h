/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types.h"

bool modules_initialized();

void initialize_modules();

void module_register(module_t& driver);

module_t const* find_module_by_class(module_class driver_type);
// module_t const* find_driver_by_id(uint32_t id);
// module_t const* find_driver_by_name(const short_name& name);
// module_t const* find_driver_by_class_and_id(module_class driver_type,
// uint32_t id); module_t const* find_driver_by_class_and_name(module_class
// driver_type, const short_name& name);
