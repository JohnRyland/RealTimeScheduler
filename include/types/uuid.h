/*
  Universally Unique Identifiers
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types/integers.h"

// [1] 5.2.4 Universally Unique Identifiers (UUIDs), page 116
struct uuid_t
{
  uint8_t          uuid[16];
};

// References:
//  [1] Advanced Configuration and Power Interface (ACPI) Specification 6.3
//      Copyright, UEFI Forum Inc, 2018-2019
//      https://uefi.org/sites/default/files/resources/ACPI_6_3_final_Jan30.pdf
