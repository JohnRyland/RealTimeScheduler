/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#if 0

struct BusDeviceFunction
{
    //uint32_t base_upper;
    uint32_t base     : 9;
    uint32_t bus      : 3;  // 8
    uint32_t device   : 5;  // 32
    uint32_t function : 3;  // 8
    uint32_t offset   : 12; // 4kb
};

static_assert(sizeof(BusDeviceFunction) == 4, "bit fields not working!");

// Physical_Address = MMIO_Starting_Physical_Address + ((Bus - MMIO_Starting_Bus) << 20 | Device << 15 | Function << 12).


struct bios_data_area_t
{
  uint16_t ext_bios_data_area_seg;
};


// ACPI - Advanced configuration and power interface
static pci_entry_t* pcie_info = nullptr;


//https://en.wikipedia.org/wiki/PCI_configuration_space
//  8-bit bus, 5-bit device, 3-bit function -> total of 16-bits for the B/D/F

pci_header_t* pcie_get_header(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pci_header_t*)(pcie_info->base_address | bus << 20 | slot << 15 | func << 12);
}


// https://wiki.osdev.org/PCI#Enumerating_PCI_Buses
uint16_t pcie_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    // Using mmio
    if (pcie_info)
        return *(uint16_t*)(pcie_info->base_address | ((bus - pcie_info->start_pci_bus_number) << 20 | slot << 15 | func << 12) | offset);

    // Else use port mapped io

    // Create configuration address as per Figure 1
    uint32_t address = (uint32_t)((uint32_t(bus) << 16) | (uint32_t(slot) << 11)
                        | (uint32_t(func) << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    outportd(0xCF8, address);
    uint32_t data = inportd(0xCFC);

    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pcie_config_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    // Using mmio
    if (pcie_info)
        return *(uint8_t*)(pcie_info->base_address | ((bus - pcie_info->start_pci_bus_number) << 20 | slot << 15 | func << 12) | offset);

    // Else use port mapped io
    if (offset & 1)
        return pcie_config_read_word(bus, slot, func, offset) >> 8;
    return pcie_config_read_word(bus, slot, func, offset) & 0xFF;
}

uint32_t getVendorAndDeviceID(uint8_t bus, uint8_t slot, uint8_t func)
{
    uint16_t vendor = pcie_config_read_word(bus, slot, func, 0);
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if (vendor != 0xFFFF) {
      uint16_t device = pcie_config_read_word(bus, slot, func, 2);
      if (device != slot)
        return ((uint32_t)vendor << 16) | (uint32_t)device;
    }
    return (uint32_t)-1;
}

uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t func) {
  return pcie_config_read_byte(bus, device, func, 14);
}
uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t func) {
    return pcie_config_read_byte(bus, device, func, 11);
}
uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t func) {
    return pcie_config_read_byte(bus, device, func, 10);
}
uint8_t getProgIF(uint8_t bus, uint8_t device, uint8_t func) {
    return pcie_config_read_byte(bus, device, func, 9);
}

/*
uint8_t getSecondaryBus(uint8_t bus, uint8_t device, uint8_t func) {
    return pcie_config_read_byte(bus, device, func, 10);
}

void checkBus(uint8_t bus);
*/

#if 0
void print_acpi_description(uint32_t vendorDevice, uint8_t , uint8_t , uint8_t )
{
  k_log_fmt(ERROR, "[ ] vendor: %x device: %x \n", vendorDevice>>16, vendorDevice & 0xFFFF);
}
#else
void print_acpi_description(uint32_t vendorDevice, uint8_t cls, uint8_t sub, uint8_t prog)
{
  const char* cls_desc = "";
  const char* sub_desc = "";
  const char* pif_desc = "";
  // const char* vendor_desc = "";
  auto* clas = pcie_desc_tree.find(cls);
  if (clas)
  {
    cls_desc = clas->data();
    auto* subcls = clas->find(sub);
    if (subcls)
    {
      sub_desc = subcls->data();
      auto* prog_if = subcls->find(prog);
      if (prog_if)
      {
        pif_desc = prog_if->data();
      }
    }
  }
  k_log_fmt(ERROR, "[ ]   PCIE - vid:pid: %x:%x - %s / %s / %s \n", vendorDevice>>16, vendorDevice & 0xFFFF, cls_desc, sub_desc, pif_desc);
}
#endif

void checkFunction(uint8_t bus, uint8_t device, uint8_t function)
{
  uint32_t vendorDevice = getVendorAndDeviceID(bus, device, function);
  uint8_t baseClass = getBaseClass(bus, device, function);
  uint8_t subClass = getSubClass(bus, device, function);
  uint8_t prog_if = getProgIF(bus, device, function);
  if ((baseClass == 0x6) && (subClass == 0x4))
  {
      k_log_fmt(ERROR, "[ ] ACPI detected secondary bus which was skipped.\n");
  //         uint8_t secondaryBus;
  //         secondaryBus = getSecondaryBus(bus, device, function);
  //         checkBus(secondaryBus);
  }

  // k_log_fmt(ERROR, "[ ] ACPI detected bus:slot:func  =  %i:%i:%i  %i,%i %i\n",
  //      bus, device, function, baseClass, subClass, prog_if);
  
  print_acpi_description(vendorDevice, baseClass, subClass, prog_if);

  /*
    0:0:0   6,0     - Host bridge
    0:1:0   3,0     - VGA controller
    0:2:0   2,0     - eth controller

    0:31:0  6,1     - ISA bridge
    0:31:2  1,6     - SATA controller
    0:31:3  12,5    - SMBus controller

            12,3,32  - SB2 EHCI controller
            12,3,0   - USB UHCI controller  

[ ] vendor: 1234:   device: 1111  description: Display Controller / VGA Compatible Controller / VGA Controller 
[ ] vendor: 8086:   device: 100E  description: Network Controller / Ethernet Controller /  
[ ] vendor: 8086:   device: 2918  description: Bridge / ISA Bridge /  
[ ] vendor: 8086:   device: 2922  description: Mass Storage Controller / Serial ATA Controller / AHCI 1.0 
[ ] vendor: 8086:   device: 2930  description: Serial Bus Controller / SMBus Controller / 
[ ] vendor: 8086:   device: 2934  description: Serial Bus Controller / USB Controller / UHCI Controller 
[ ] vendor: 8086:   device: 2935  description: Serial Bus Controller / USB Controller / UHCI Controller 
[ ] vendor: 8086:   device: 2936  description: Serial Bus Controller / USB Controller / UHCI Controller 
[ ] vendor: 8086:   device: 293A  description: Serial Bus Controller / USB Controller / EHCI (USB2) Controller 
[ ] vendor: 8086:   device: 29C0  description: Bridge / Host Bridge /  

*/
}


void checkDevice(uint8_t bus, uint8_t device)
{
     uint8_t function = 0;
     uint32_t vendorDeviceID = getVendorAndDeviceID(bus, device, function);
     if (vendorDeviceID == 0xFFFFFFFF)
        return; // Device doesn't exist
     checkFunction(bus, device, function);
     uint8_t headerType = getHeaderType(bus, device, function);
     if( (headerType & 0x80) != 0) {
         // It's a multi-function device, so check remaining functions
         for (function = 1; function < 8; function++) {
             if (getVendorAndDeviceID(bus, device, function) != 0xFFFFFFFF) {
                 checkFunction(bus, device, function);
             }
         }
     }
}

void checkBus(uint8_t bus)
{
    for (uint8_t device = 0; device < 32; device++)
    {
        checkDevice(bus, device);
    }
}

#if 0
 void checkAllBuses() {
     uint16_t bus;
     uint8_t device;
 
     for (bus = 0; bus < 256; bus++) {
         for (device = 0; device < 32; device++) {
             checkDevice(bus, device);
         }
     }
 }
#elif 1

 void checkAllBuses()
 {
   uint8_t bus0 = pcie_info->start_pci_bus_number;
   uint8_t headerType = pcie_get_header(0, 0, 0)->header_type;
   for (uint8_t bus = bus0; bus < pcie_info->end_pci_bus_number; bus++)
   {
     if (pcie_get_header(0, 0, bus)->vendor_id != 0xFFFF)
     {
       for (uint8_t device = 0; device < 32; device++)
       {
         auto* ptr = pcie_get_header(bus, device, 0);
         if (ptr->vendor_id != 0xFFFF) {
           for (int function = 0; function < 8; function++) {
             auto* ptr2 = pcie_get_header(bus, device, function);
             if (ptr2->vendor_id != 0xFFFF && (function == 0 || (ptr->header_type & 0x80) != 0))
             {
               k_log_fmt(SUCCESS, "[X]   PCIE - vid:pid: %x:%x - ", ptr2->vendor_id, ptr2->device_id);
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
               if (/* ptr2->class_id == 0x6 && ptr2->subclass_id == 0x4 && */ ptr2->header_type == 0x1)
               {
                 k_log_fmt(SUCCESS, "[X]   PCIE - detected secondary bus %i.\n", ptr2->SecondaryBusNumber);
                 // checkBus(ptr2->SecondaryBusNumber);
               }
             }
           }
         }
       }
     }
     if ((headerType & 0x80) == 0)
       // Single PCI host controller, finish after first iteration
       break;
     // Multiple PCI host controllers, continue iterating
   }
 }

#else
 void checkAllBuses() { 
     uint8_t headerType = getHeaderType(0, 0, 0);
     if ((headerType & 0x80) == 0) {
         // Single PCI host controller
         checkBus(0);
     } else {
         // Multiple PCI host controllers
         for (uint8_t bus = 0; bus < 8; bus++) {
             if (getVendorAndDeviceID(0, 0, bus) != 0xFFFFFFFF)
                break;
             checkBus(bus);
         }
     }
}
#endif

void initialize_acpi()
{
  uint32_t acpi_base_address = 0;
  int processors = 0;
  uint8_t core_ids[128];

  //clrscr();
  if (!find_acpi(acpi_base_address))
    k_log_fmt(ERROR, "[ ] ACPI base address not found.\n");
  else
  {
    ACPI_RSDR* rsdr_ptr = (ACPI_RSDR*)acpi_base_address;
    ACPI_RSDT* rsdt_ptr = (ACPI_RSDT*)rsdr_ptr->rsdt_address;
    if (!FourCC::matches(rsdt_ptr->header.sig, "RSDT"))
    {
      k_log_fmt(ERROR, "[ ] ACPI RSDT not found.\n");
    }
    else
    {
      k_log_fmt(SUCCESS, "[X] ACPI tables found, parsing\n");
      int entry_count = (rsdt_ptr->header.length - 36) / 4;
      for (int i = 0; i < entry_count; ++i)
      {
        char sig[5];
        for (int j = 0; j < 4; j++)
          sig[j] = ((char*)rsdt_ptr->entries[i])[j];
        sig[4] = 0;

        // [1] Table 5-29 DESCRIPTION_HEADER Signatures for tables defined by ACPI, page 120

        if (FourCC::matches(sig, "MCFG"))
        {
            k_log_fmt(SUCCESS, "[X] ACPI - Parsing PCIE information.\n");

            ACPI_MCFG* pcie_cfg = (ACPI_MCFG*)rsdt_ptr->entries[i];
            size_t pcie_ent_count = (pcie_cfg->header.length - sizeof(ACPI_MCFG)) / sizeof(pci_entry_t);

            for (size_t e = 0; e < pcie_ent_count; e++)
            {
                // TODO: what if there is more than 1?
                pcie_info = &pcie_cfg->entries[e];

                auto ent = pcie_cfg->entries[e];
                auto ba = ent.base_address;
                ba = ba + 1;
                //k_log_fmt(SUCCESS, " grp %i bus range: %i - %i at 0x%x %x \n", ent.pci_segment_group_number,
                //    ent.start_pci_bus_number, ent.end_pci_bus_number, (uint32_t)(ent.base_address >> 32), (uint32_t)ent.base_address);
            }

            checkAllBuses();

            k_log_fmt(SUCCESS, "[X] ACPI - Parsed PCIE information.\n");
        }
        else if (FourCC::matches(sig, "APIC"))
        {
            k_log_fmt(SUCCESS, "[X] ACPI - Parsing APIC information.\n");

            ACPI_APIC* apic_data = (ACPI_APIC*)rsdt_ptr->entries[i];
            size_t remaining_bytes = apic_data->header.length - sizeof(ACPI_APIC);
            uint8_t* entry_ptr = &apic_data->entries[0];
            while (remaining_bytes)
            {
                apic_ent_t* entry = (apic_ent_t*)entry_ptr;

                static const char* apic_types[] = {
                    "processor local", "i/o", "interrupt source override", "NMI source", "local NMI", "local address override", "processor local b", "-"
                };

                k_log_fmt(SUCCESS, "[X]   APIC - type %i (%s apic) found.\n", entry->typ, apic_types[entry->typ & 7]);

                if (entry->typ == 0)
                {
                  core_ids[processors] = entry->id;
                  processors++;
                }

                entry_ptr += entry->len;
                remaining_bytes -= entry->len;
            }
            k_log_fmt(SUCCESS, "[X]   APIC - Found %i processors/cores.\n", processors);

            k_log_fmt(SUCCESS, "[X] ACPI - Parsed APIC information.\n");
        }
        else if (FourCC::matches(sig, "FACP"))
        {
            k_log_fmt(SUCCESS, "[X] ACPI - Parsing FADT information.\n");
            ACPI_FADT* fadt_data = (ACPI_FADT*)rsdt_ptr->entries[i];
            k_log_fmt(SUCCESS, "[X]   APIC - FADT  enable: %i, disable: %i.\n", fadt_data->AcpiEnable, fadt_data->AcpiEnable);
            k_log_fmt(SUCCESS, "[X] ACPI - Parsed FADT information.\n");
        }
        else if (FourCC::matches(sig, "HPET"))
        {            
            k_log_fmt(SUCCESS, "[X] ACPI - Parsing HPET information.\n");
            ACPI_HPET* hpet_data = (ACPI_HPET*)rsdt_ptr->entries[i];
            k_log_fmt(SUCCESS, "[X]   APIC - HPET #%i, vendor: %x, size: %i.\n", hpet_data->hpet_number, hpet_data->pci_vendor_id, hpet_data->counter_size);
            k_log_fmt(SUCCESS, "[X] ACPI - Parsed HPET information.\n");
        }
        else if (FourCC::matches(sig, "WAET"))
        {
            k_log_fmt(SUCCESS, "[X] ACPI - Parsing WAET information.\n");
            ACPI_WAET* waet_data = (ACPI_WAET*)rsdt_ptr->entries[i];
            k_log_fmt(SUCCESS, "[X]   APIC - WAET flags: %x.\n", waet_data->emulated_device_flags);
            k_log_fmt(SUCCESS, "[X] ACPI - Parsed WAET information.\n");
        }
        else
        {
            k_log_fmt(WARNING, "[ ] ACPI - Found %s table.\n", sig);
        }
      }
    }
  }

  if (!processors)
    processors++;

  for (int i = 0; i < processors; i++)
  {

  }
  // k_log_fmt(ERROR, "[ ] Found %i processors/cores.\n", processors);
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

#endif
