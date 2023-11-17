/*
  ACPI Initialization
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types/pci_device.h"
#include "types/uuid.h"

void initialize_acpi();

// [1] 5.2.3.2 Generic Address Structure (GAS), page 115
struct ACPI_GAS_t
{
  uint8_t          address_space;
  uint8_t          bit_width;
  uint8_t          bit_offset;
  uint8_t          access_size;
  uint64_t         address;
} __attribute__ ((packed));

// [1] 5.2.5.3 Root System Description Pointer (RSDP) Structure, page 118
struct ACPI_RSDR_t
{
  char             signature[8]; // "RSD PTR"
  uint8_t          checksum;
  char             oem_id[6];
  uint8_t          revision;
  uint32_t         rsdt_address;  // revision 1.0

  // When revision is 2.0, use the xsdt 64-bit version instead
  uint32_t         length;
  uint64_t         xsdt_address;
  uint8_t          extended_checksum;
  char             reserved[3];
}; // no need to mark packed

// [1] 5.2.6 System Description Table Header, page 119
struct ACPI_header_t
{
  char             signature[4]; // "RSDT" | "FACP" | "APIC" etc
  uint32_t         length;
  uint8_t          revision;
  uint8_t          checksum;
  char             oem_id[6];
  uint64_t         oem_table_id;
  uint32_t         oem_revision;
  uint32_t         creator_id;
  uint32_t         creator_revision;
}; // no need to mark packed

// [1] 5.2.7 Root System Description Table (RSDT), page 124
struct ACPI_RSDT_t
{
  ACPI_header_t    header;     // sig == "RSDT"

  uint32_t         entries[];  // List of pointers to other tables
};

// [1] 5.2.8 Extended System Description Table (XSDT), page 125
struct ACPI_XSDT_t
{
  ACPI_header_t    header;     // sig == "XSDT"

  uint64_t         entries[];  // List of pointers to other tables
};

// [1] 5.2.9 Fixed ACPI Description Table (FADT), page 126
//  see also: https://wiki.osdev.org/FADT
struct ACPI_FADT_t
{
  ACPI_header_t    header;     // sig == "FACP"

  uint32_t         firmware_control;
  uint32_t         dsdt;

  // field used in ACPI 1.0; no longer in use, for compatibility only
  uint8_t          reserved;

  uint8_t          preferred_power_management_profile;
  uint16_t         sci_interrupt;
  uint32_t         smi_command_port;
  uint8_t          acpi_enable;
  uint8_t          acpi_disable;
  uint8_t          s4_bios_req;
  uint8_t          pstate_control;
  uint32_t         pm_1a_event_block;
  uint32_t         pm_1b_event_block;
  uint32_t         pm_1a_control_block;
  uint32_t         pm_1b_control_block;
  uint32_t         pm_2_control_block;
  uint32_t         pm_timer_block;
  uint32_t         gpe_0_block;
  uint32_t         gpe_1_block;
  uint8_t          pm_1_event_length;
  uint8_t          pm_1_control_length;
  uint8_t          pm_2_control_length;
  uint8_t          pm_timer_length;
  uint8_t          gpe_0_length;
  uint8_t          gpe_1_length;
  uint8_t          gpe_1_base;
  uint8_t          cstate_control;
  uint16_t         worst_c2_latency;
  uint16_t         worst_c3_latency;
  uint16_t         flush_size;
  uint16_t         flush_stride;
  uint8_t          duty_offset;
  uint8_t          duty_width;
  uint8_t          day_alarm;
  uint8_t          month_alarm;
  uint8_t          century;

  // reserved in ACPI 1.0; used since ACPI 2.0+
  uint16_t         boot_architecture_flags;

  uint8_t          reserved_2;
  uint32_t         flags;

  // 12 byte structure; see below for details
  ACPI_GAS_t       reset_register;

  uint8_t          reset_value;
  uint8_t          reserved_3[3];

  // 64bit pointers - Available on ACPI 2.0+
  uint64_t         x_firmware_control;
  uint64_t         x_dsdt;

  ACPI_GAS_t       x_pm_1a_event_block;
  ACPI_GAS_t       x_pm_1b_event_block;
  ACPI_GAS_t       x_pm_1a_control_block;
  ACPI_GAS_t       x_pm_1b_control_block;
  ACPI_GAS_t       x_pm_2_control_block;
  ACPI_GAS_t       x_pm_timer_block;
  ACPI_GAS_t       x_gpe_0_block;
  ACPI_GAS_t       x_gpe_1_block;
} __attribute__ ((packed));

// [1] 5.2.12 Multiple APIC Description Table (MADT), page 151
//  see also: https://wiki.osdev.org/MADT
struct ACPI_MADT_t
{
  ACPI_header_t    header;                   // sig == "APIC"
  
  uint32_t         local_interrupt_controller_address; // local_apic_address
  uint32_t         flags;

  // Multiple entries of APIC_entry_t which are variable size
  uint8_t          entries[];
};
using ACPI_APIC_t = ACPI_MADT_t;

// [1] 5.2.12.2 Processor Local APIC Structure, page 154
struct APIC_entry_t
{
  uint8_t          entry_type;               // Table 5-45 Interrupt Controller Structure Types
  uint8_t          entry_length;
  uint8_t          uid;                      // APIC Processor UID
  uint8_t          id;                       // APIC local processor ID
  uint32_t         flags;
  uint8_t          data[];
};

// [2] 3.2.4 Intel High Precision Event Timers (HPET) Description Table, page 30
struct ACPI_HPET_t
{
  ACPI_header_t    header;                   // sig == "HPET"

  // 32-bit event timer block
  uint8_t          hardware_rev_id;
  uint8_t          comparator_count:5;
  uint8_t          counter_size:1;           // COUNT_SIZE_CAP
  uint8_t          reserved:1;
  uint8_t          legacy_replacement:1;     // IRQ routing capable
  uint16_t         pci_vendor_id;

  ACPI_GAS_t         base_address;
  uint8_t          hpet_number;
  uint16_t         minimum_tick;             // clock tick units
  uint8_t          page_protection;          // and OEM attrib
};

// [3] Table 1. Windows ACPI Emulated Devices Table, page 5
struct ACPI_WAET_t
{
  ACPI_header_t    header;                   // sig == "WAET"

  uint32_t         emulated_device_flags;
};

// Boot Graphics Record Table, https://wiki.osdev.org/BGRT
struct ACPI_BGRT_t
{
  ACPI_header_t    header;                   // sig == "BGRT"

  uint16_t         version_id;               // must be 1
  uint8_t          status;                   // bit-0: displayed, bits-1,2: orientation offset
  uint8_t          image_type;               // 0 = bitmap
  uint64_t         image_address;
  uint32_t         image_x_offset;
  uint32_t         image_y_offset;
};

// References:
//
//  [1] Advanced Configuration and Power Interface (ACPI) Specification 6.3
//      Copyright, UEFI Forum Inc, 2018-2019
//      https://uefi.org/sites/default/files/resources/ACPI_6_3_final_Jan30.pdf
//
//  [2] IA-PC HPET (High Precision Event Timers) Specification 1.0a
//      Copyright, Intel Corporation, 1999-2004
//      https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
//
//  [3] Windows ACPI Emulated Devices Table Format 1.0
//      Copyright, Microsoft Corporation, 2009
//      https://download.microsoft.com/download/7/E/7/7E7662CF-CBEA-470B-A97E-CE7CE0D98DC2/WAET.docx
//
// Not referenced:
//
//  [4] Advanced Configuration and Power Interface (ACPI) Specification 6.5
//      Copyright, UEFI Forum Inc, 2022
//      https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf
