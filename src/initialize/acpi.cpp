/*
  ACPI Initialization
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "kernel/debug_logger.h"
#include "types/acpi.h"
#include "types/pcie.h"
#include "common/fourcc.hpp"

// (the real mode segment pointer to it is located at 0x40E), or in the memory region from 0x000E0000 to 0x000FFFFF
// (the main BIOS area below 1 MB). To find the table, the Operating System has to
// find the "RSD PTR " string (notice the last space character) in one of the two areas.
// This signature is always on a 16 byte boundary.
static bool scan_for_sig(uint32_t start_address, uint32_t len, uint32_t& acpi_base_address)
{
  static const char cmp_str[] = "RSD PTR ";
  for (acpi_base_address = start_address & 0xFFFFFFF0; acpi_base_address < start_address + len + 16; acpi_base_address += 8)
  {
    bool found = true;
    for (uint32_t i = 0; i < 8; i++)
      if (*(char*)(acpi_base_address + i) != cmp_str[i])
        found = false;
    if (found)
    {
      // TODO: check checksums / crc just in case
      return true;
    }
  }
  return false;
}

// [1] 5.2.5.1 Finding the RSDP on IA-PC Systems, page 117
static bool find_acpi(uint32_t& acpi_base_address)
{
  // This is the legacy way by searching memory (Should be mainly searching BIOS ROM memory that is mapped to specific address
  // ranges in the address space, usually in the lower 1MB of memory for legacy 16-bit systems, original IBM PC spec BIOS stuff)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
  uint16_t ext_bios_data_area_segment = *(uint16_t*)0x040E;  // BIOS Data Area value points to the segment of the Extended BIOS Data Area
#pragma GCC diagnostic pop
  if (!scan_for_sig(uint32_t(ext_bios_data_area_segment) << 4, 0x0000FFFF, acpi_base_address))
    if (!scan_for_sig(0x000E0000, 0x000FFFFF - 0x000E0000, acpi_base_address)) // These address ranges should avoid else where in the code
      return false;
  return true;
}

// [1] 5.2.5.2 Finding the RSDP on UEFI Enabled Systems, page 117
//static
bool uefi_find_acpi(uint32_t& acpi_base_address)
{
  // TODO: find it the EFI way assuming UEFI booted. First, how do we detect if we booted from UEFI ?
  //       and how to setup qemu for this?
  // EFI GUID for finding RSDP of ACPI 1.0
  //  eb9d2d30-2d88-11d3-9a16-0090273fc14d.
  // EFI GUID for finding RSDP of ACPI 2.0
  //  8868e871-e4f1-11d3-bc22-0080c73c8881.
  acpi_base_address = -1;
  return false;
}

// [1] 5.2.10.1 Global Lock, page 146
/*
AcquireGlobalLock:
        mov ecx, GlobalLock                ; ecx = Address of Global Lock in FACS
acq10:  mov eax, [ecx]                     ; Get current value of Global Lock
        mov edx, eax
        and edx, not 1                     ; Clear pending bit
        bts edx, 1                         ; Check and set owner bit
        adc edx, 0                         ; If owned, set pending bit
        lock cmpxchg dword ptr[ecx], edx   ; Attempt to set new value
        jnz short acq10                    ; If not set, try again
        cmp dl, 3                          ; Was it acquired or marked pending?
        sbb eax, eax                       ; acquired = -1, pending = 0
        ret
ReleaseGlobalLock:
       mov ecx, GlobalLock                  ; ecx = Address of Global Lock in FACS
rel10: mov eax, [ecx]                       ; Get current value of Global Lock
       mov edx, eax
       and edx, not 03h                     ; Clear owner and pending field
       lock cmpxchg dword ptr[ecx], edx     ; Attempt to set it
       jnz short rel10                      ; If not set, try again
       and eax, 1                           ; Was pending set?
       ; if one is returned (we were pending) the caller must signal that the
       ; lock has been released using either GBL_RLS or BIOS_RLS as appropriate
       ret
*/

extern void k_msleep(int milliseconds);
extern uint8_t smp_core_char1;
extern uint8_t smp_core_char2;
extern uint32_t print_32_line_no;

// Reference:
//   https://wiki.osdev.org/SMP
void startMultiCores(ACPI_APIC_t* apic_data, uint8_t* coreids, int cores)
{
  uint8_t* lapic_ptr = (uint8_t*)apic_data->local_interrupt_controller_address;
  // BSP -> Bootstrap Processor (in this context it doesn't mean board-support-package which is also BSP, ditto for binary-space-partitioning)
  uint8_t bspid;
  // get the BSP's Local APIC ID
  __asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );

  // for each Local APIC ID we do...
  for (int i = 0; i < cores; i++)
  {
    // do not start BSP, that's already running this code
    if (coreids[i] == bspid)
    {
      k_log_fmt(SUCCESS, "[X]   Bootstrap Processor %i\n", bspid);
      continue;
    }
    // send INIT IPI
    *((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;                                                                             // clear APIC errors
    *((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);         // select AP
    *((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x00C500;          // trigger INIT IPI
    do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));         // wait for delivery
    *((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);         // select AP
    *((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x008500;          // deassert
    do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));         // wait for delivery
  }
  k_msleep(10);                                                                                                                 // wait 10 msec
  for (int i = 0; i < cores; i++)
  {
    print_32_line_no = 0;
    smp_core_char1 = '0' + (i/10) % 10;
    smp_core_char2 = '0' + (i/ 1) % 10;
    // send STARTUP IPI (twice)
  	for(int j = 0; j < 2; j++)
    {
      *((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;                                                                     // clear APIC errors
      *((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24); // select AP
      *((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff0f800) | 0x000608;  // trigger STARTUP IPI for 0800:0000
      k_msleep(50);                                                                                                       // wait 200 usec
	  do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12)); // wait for delivery
    }
  }
  k_msleep(50);                                                                                                           // wait 200 usec

  // baremetal-OS loader code called pure64 has something similar to the above in assembler
  // Reference:
  //   https://github.com/ReturnInfinity/Pure64/blob/master/src/init/smp.asm
  // Some of the numbers are slightly different, but kind of the same process is being used.
  // Pure64 doesn't look like it reads the memory before modifying and writing it, so possibly there are some shortcuts to the above.
}

int checkAPICs(ACPI_APIC_t* apic_data)
{
  uint8_t core_ids[128];
  int processors = 0;
  size_t remaining_bytes = apic_data->header.length - sizeof(ACPI_APIC_t);
  uint8_t* entry_ptr = &apic_data->entries[0];
  while (remaining_bytes)
  {
    APIC_entry_t* entry = (APIC_entry_t*)entry_ptr;

    static const char* apic_types[] = {
      "processor local", "i/o", "interrupt source override", "NMI source", "local NMI", "local address override", "processor local b", "-"
    };

    k_log_fmt(SUCCESS, "[X]   APIC - type %i (%s apic) found.\n", entry->entry_type, apic_types[entry->entry_type & 7]);

    if (entry->entry_type == 0)
    {
      core_ids[processors] = entry->id;
      processors++;
    }

    entry_ptr += entry->entry_length;
    remaining_bytes -= entry->entry_length;
  }

  if (!processors)
    processors++;

  startMultiCores(apic_data, core_ids, processors);

  return processors;
}

void initialize_acpi()
{
  uint32_t acpi_base_address = 0;
  if (!find_acpi(acpi_base_address))
  {
    k_log_fmt(ERROR, "[ ] ACPI base address not found.\n");
    return;
  }

  ACPI_RSDR_t* rsdr_ptr = (ACPI_RSDR_t*)acpi_base_address;
  ACPI_RSDT_t* rsdt_ptr = (ACPI_RSDT_t*)rsdr_ptr->rsdt_address;
  if (!FourCC::matches(rsdt_ptr->header.signature, "RSDT"))
  {
    k_log_fmt(ERROR, "[ ] ACPI RSDT not found.\n");
    return;
  }

  k_log_fmt(SUCCESS, "[X] ACPI tables found, parsing\n");
  int entry_count = (rsdt_ptr->header.length - 36) / 4;
  for (int i = 0; i < entry_count; ++i)
  {
    // [1] Table 5-29 DESCRIPTION_HEADER Signatures for tables defined by ACPI, page 120
    uint8_t* signature = (uint8_t*)rsdt_ptr->entries[i];
    if (FourCC::matches(signature, "MCFG"))
    {
      k_log_fmt(SUCCESS, "[X] ACPI - Parsing PCIE information.\n");
      ACPI_MCFG_t* mcfg_data = (ACPI_MCFG_t*)rsdt_ptr->entries[i];
      checkPCIeConfig(mcfg_data);
      k_log_fmt(SUCCESS, "[X] ACPI - Parsed PCIE information.\n");
    }
    else if (FourCC::matches(signature, "APIC"))
    {
      k_log_fmt(SUCCESS, "[X] ACPI - Parsing APIC information.\n");
      ACPI_APIC_t* apic_data = (ACPI_APIC_t*)rsdt_ptr->entries[i];
      int processors = checkAPICs(apic_data);
      k_log_fmt(SUCCESS, "[X]   APIC - Found %i processors/cores.\n", processors);
      k_log_fmt(SUCCESS, "[X] ACPI - Parsed APIC information.\n");
    }
    else if (FourCC::matches(signature, "FACP"))
    {
      k_log_fmt(SUCCESS, "[X] ACPI - Parsing FADT information.\n");
      ACPI_FADT_t* fadt_data = (ACPI_FADT_t*)rsdt_ptr->entries[i];
      k_log_fmt(SUCCESS, "[X]   APIC - FADT  enable: %i, disable: %i.\n", fadt_data->acpi_enable, fadt_data->acpi_disable);
      k_log_fmt(SUCCESS, "[X] ACPI - Parsed FADT information.\n");
    }
    else if (FourCC::matches(signature, "HPET"))
    {            
      k_log_fmt(SUCCESS, "[X] ACPI - Parsing HPET information.\n");
      ACPI_HPET_t* hpet_data = (ACPI_HPET_t*)rsdt_ptr->entries[i];
      k_log_fmt(SUCCESS, "[X]   APIC - HPET #%i, vendor: %x, size: %i.\n", hpet_data->hpet_number, hpet_data->pci_vendor_id, hpet_data->counter_size);
      k_log_fmt(SUCCESS, "[X] ACPI - Parsed HPET information.\n");
    }
    else if (FourCC::matches(signature, "WAET"))
    {
      k_log_fmt(SUCCESS, "[X] ACPI - Parsing WAET information.\n");
      ACPI_WAET_t* waet_data = (ACPI_WAET_t*)rsdt_ptr->entries[i];
      k_log_fmt(SUCCESS, "[X]   APIC - WAET flags: %x.\n", waet_data->emulated_device_flags);
      k_log_fmt(SUCCESS, "[X] ACPI - Parsed WAET information.\n");
    }
    else
    {
      k_log_fmt(WARNING, "[ ] ACPI - Found %s table.\n", signature);
    }
  }
}
