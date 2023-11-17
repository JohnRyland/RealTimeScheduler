/*
  PCI-E Device
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "integers.h"

// TODO: comment where this comes from - should be PCIe spec
// Some details here:
//   https://wiki.osdev.org/PCI_Express
// Some inconsistency with this and other sources. Need to find
// the canonical source of the spec for this
struct PCIe_function_t
{
  uint16_t         vendor_id;
  uint16_t         device_id;

  uint16_t         command;
  uint16_t         status;

  uint8_t          revision_id;
  uint8_t          function_id;
  uint8_t          subclass_id;
  uint8_t          class_id;

  uint8_t          cache_line_size;          // in 32-bit units
  uint8_t          Latency_timer;            // in PCI bus clocks
  uint8_t          header_type;
  uint8_t          built_in_self_test_status;

  // additional fields depending on header_type
  union
  {
    struct
    {
      uint32_t	cardbus_cis;			/* CardBus CIS pointer */
      uint16_t	subsystem_id;			/* subsystem (add-in card) id */
      uint16_t	subsystem_vendor_id;	/* subsystem (add-in card) vendor id */
      uint32_t	rom_base;				/* rom base address, viewed from host */
      uint32_t	rom_base_pci;			/* rom base addr, viewed from pci */
      uint32_t	rom_size;				/* rom size */
      uint32_t	base_registers[6];		/* base registers, viewed from host */
      uint32_t	base_registers_pci[6];	/* base registers, viewed from pci */
      uint32_t	base_register_sizes[6];	/* size of what base regs point to */
      uint8_t	base_register_flags[6];	/* flags from base address fields */
      uint8_t	interrupt_line;			/* interrupt line */
      uint8_t	interrupt_pin;			/* interrupt pin */
      uint8_t	min_grant;				/* burst period @ 33 Mhz */
      uint8_t	max_latency;			/* how often PCI access needed */
    } header_type_0;
    struct
    {
      /*
      uint32_t         base_addresses[2];
      uint8_t          primary_bus_number;
      uint8_t          secondary_bus_number;
      uint8_t          subordinate_bus_number;
      */

      uint32_t	base_registers[2];		/* base registers, viewed from host */
      //uint32_t	base_registers_pci[2];	/* base registers, viewed from pci */
      //uint32_t	base_register_sizes[2];	/* size of what base regs point to */
      //uint8_t	    base_register_flags[2];	/* flags from base address fields */
      uint8_t	primary_bus;
      uint8_t	secondary_bus;
      uint8_t	subordinate_bus;
      uint8_t	secondary_latency;
      uint8_t	io_base;
      uint8_t	io_limit;
      uint16_t	secondary_status;
      uint16_t	memory_base;
      uint16_t	memory_limit;
      uint16_t  prefetchable_memory_base;
      uint16_t  prefetchable_memory_limit;
      uint32_t	prefetchable_memory_base_upper32;
      uint32_t	prefetchable_memory_limit_upper32;
      uint16_t	io_base_upper16;
      uint16_t	io_limit_upper16;
      uint32_t	rom_base;				/* rom base address, viewed from host */
      uint32_t	rom_base_pci;			/* rom base addr, viewed from pci */
      uint8_t	interrupt_line;			/* interrupt line */
      uint8_t	interrupt_pin;			/* interrupt pin */
      uint16_t	bridge_control;
      uint16_t	subsystem_id;			/* subsystem (add-in card) id */
      uint16_t	subsystem_vendor_id;	/* subsystem (add-in card) vendor id */
    } header_type_1;
    struct
    {
      uint16_t	subsystem_id;			/* subsystem (add-in card) id */
      uint16_t	subsystem_vendor_id;	/* subsystem (add-in card) vendor id */
    } header_type_2;
  } data;

  uint8_t        reserved[4096-16 - sizeof(data.header_type_0)];
};

static_assert(sizeof(PCIe_function_t) == 4096, "pcie_function needs to be 4096 bytes");
