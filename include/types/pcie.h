/*
  PCIE Initialization
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types/pci_device.h"
#include "types/acpi.h"

// [1] Table 4-3: Memory Mapped Enhanced Configuration Space Base Address Allocation Structure, page 42
struct PCI_entry_t
{
  uint64_t         base_address;
  uint16_t         pci_segment_group_number; // corresponds to _SEG of host bridge device in ACPI
  uint8_t          start_pci_bus_number;
  uint8_t          end_pci_bus_number;
  uint32_t         reserved;
};

// [1] 4.1.2. PCI Memory-mapped Configuration (MCFG) Table Description, page 41
struct ACPI_MCFG_t
{
  ACPI_header_t    header;                   // sig == "MCFG"

  uint8_t          reserved[8];
  PCI_entry_t      entries[];
};

// https://en.wikipedia.org/wiki/PCI_configuration_space
using PCIe_device_t = PCIe_function_t[8];
using PCIe_bus_t    = PCIe_device_t[32];
using PCIe_config_t = PCIe_bus_t[256];

//  8-bit bus, 5-bit device, 3-bit function -> total of 16-bits for the B/D/F
PCIe_function_t* get_pcie_function(PCIe_config_t* pci_cfg, uint8_t bus, uint8_t device, uint8_t function);

void checkPCIeConfig(ACPI_MCFG_t* mcfg_data);

// References:
//
//  [1] PCI Firmware Specification Revision 3.2
//      Copyright, PCI-SIG, 1992-2015
//      https://github.com/Puqiyuan/books/blob/9e15273f001453b8bedb3c91bc9bdc2992f082d5/spec/pci/8_PCI_Firmware_v3.2_01-26-2015_ts_clean_Firmware_Final.pdf
//
// Not referenced:
//
//  [2] PCI Express® 2.0 Base Specification Revision 0.9
//      Copyright, PCI-SIG, 2002-2006
//      https://community.intel.com/cipcp26785/attachments/cipcp26785/fpga-intellectual-property/8220/1/PCI_Express_Base_Specification_v20.pdf
