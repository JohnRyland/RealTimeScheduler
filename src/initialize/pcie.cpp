/*
  ACPI Initialization
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "types/acpi.h"
#include "types/pcie.h"
#include "types/pci_descriptors.h"
#include "kernel/debug_logger.h"

//  8-bit bus, 5-bit device, 3-bit function -> total of 16-bits for the B/D/F
PCIe_function_t* get_pcie_function(PCIe_config_t* pci_cfg, uint8_t bus, uint8_t slot, uint8_t func)
{
  return &(*pci_cfg)[bus][slot][func];
  // TODO: check details on starting_bus, if infact subtract or add - seems like should be adding and not subtracting, but not sure.
  // Physical_Address = MMIO_Starting_Physical_Address + ((Bus - MMIO_Starting_Bus) << 20 | Device << 15 | Function << 12).
}

/*
[ ] vendor: 8086:   device: 100E  description: Network Controller / Ethernet Controller /  
[ ] vendor: 8086:   device: 10D3  description: Network Controller / Ethernet Controller /  
[ ] vendor: 8086:   device: 2922  description: Mass Storage Controller / Serial ATA Controller / AHCI 1.0 
[ ] vendor: 8086:   device: 2936  description: Serial Bus Controller / USB Controller / UHCI Controller 
*/

void checkBus(PCIe_config_t* pci_cfg, uint8_t bus)
{
  for (uint8_t device = 0; device < 32; device++)
  {
    auto* ptr = get_pcie_function(pci_cfg, bus, device, 0);
    if (ptr->vendor_id != 0xFFFF)
    {
      for (int function = 0; function < 8; function++)
      {
        auto* ptr2 = get_pcie_function(pci_cfg, bus, device, function);
        if (ptr2->vendor_id != 0xFFFF && (function == 0 || (ptr->header_type & 0x80) != 0))
        {
          k_log_fmt(SUCCESS, "[X]   PCIE - path: %i/%i/%i vid:pid: %x:%x - ", bus, device, function, ptr2->vendor_id, ptr2->device_id);
          auto* clas = pcie_desc_tree.find(ptr2->class_id);
          if (clas)
          {
            k_log_fmt(SUCCESS, "%s", clas->data());
            auto* subcls = clas->find(ptr2->subclass_id);
            if (subcls)
            {
              k_log_fmt(SUCCESS, " / %s", subcls->data());
              auto* prog_if = subcls->find(ptr2->function_id);
              if (prog_if)
              k_log_fmt(SUCCESS, " / %s", prog_if->data());
            }
          }
          k_log_fmt(SUCCESS, " \n");
          // Secondary PCI bus scanning
          if (ptr2->class_id == 0x6 && ptr2->subclass_id == 0x4 && ptr2->header_type == 0x1)
          {
            k_log_fmt(SUCCESS, "[X]   PCIE - detected secondary bus %i.\n", ptr2->data.header_type_1.secondary_bus);
            if (ptr2->data.header_type_1.secondary_bus > bus)
            {
                PCIe_config_t* bus_pci_cfg = pci_cfg;
                // bus_pci_cfg = (pcie_config*)(ptr2->BaseAddresses[0]);
                checkBus(bus_pci_cfg, ptr2->data.header_type_1.secondary_bus);
            }
          }
        }
      }
    }
  }
}

void checkAllBuses(PCI_entry_t* pcie_info)
{
  PCIe_config_t* pci_cfg = (PCIe_config_t*)pcie_info->base_address;
  uint8_t headerType = get_pcie_function(pci_cfg, 0, 0, 0)->header_type;
  for (uint8_t bus = pcie_info->start_pci_bus_number; bus < pcie_info->end_pci_bus_number; bus++)
  {
    checkBus(pci_cfg, bus);
    if ((headerType & 0x80) == 0)
      // Single PCI host controller, finish after first iteration
      break;
    // Multiple PCI host controllers, continue iterating
  }
}

void checkPCIeConfig(ACPI_MCFG_t* mcfg_data)
{
  size_t pcie_ent_count = (mcfg_data->header.length - sizeof(ACPI_MCFG_t)) / sizeof(PCI_entry_t);
  for (size_t e = 0; e < pcie_ent_count; e++)
  {
    k_log_fmt(SUCCESS, "[X]   PCIE - Checking buses for entry #%i.\n", e);
    checkAllBuses(&mcfg_data->entries[e]);
  }
}

void print_desc_tree()
{
  const char* cls_desc = "";
  const char* sub_desc = "";
  const char* pif_desc = "";
  for (int i = 0; i < pcie_desc_tree.count(); i++)
  {
    auto& clas = pcie_desc_tree.branches()[i];
    cls_desc = clas.data();
    int sub_cls_cnt = clas.count();
    if (!sub_cls_cnt)
      k_log_fmt(ERROR, "[ ] description: %s / -- / -- \n", cls_desc);
    for (int j = 0; j < sub_cls_cnt; j++)
    {
      auto& subcls = clas.branches()[j];
      sub_desc = subcls.data();
      if (!subcls.count())
        k_log_fmt(ERROR, "[ ] description: %s / %s / -- \n", cls_desc, sub_desc);
      for (int k = 0; k < subcls.count(); k++)
      {
        auto& progif = subcls.branches()[k];
        pif_desc = progif.data();
        k_log_fmt(ERROR, "[ ] description: %s / %s / %s \n", cls_desc, sub_desc, pif_desc);
      }
    }
  }
}

void initialize_pcie()
{
//  checkAllBuses();
  // print_desc_tree();
}

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
