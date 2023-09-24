/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "interrupts.h"
#include "../../bootloader/i386/include/x86.h"

static
bool load()
{
  return true;
}

static
bool unload()
{
  return true;
}

// This is written in assembler and has a bunch of trampolines
extern "C" void load_idt();

static
void setup_tables()
{
#ifndef _MACOS
  load_idt();
#endif
}

static
void remap_irqs()
{
  outportb(0x20, 0x11);  // starts the initialization sequence (in cascade mode)
  outportb(0xA0, 0x11);

  outportb(0x21, 0x20);  // set the IVT entry for the IRQs
  outportb(0xA1, 0x28);

  outportb(0x21, 0x04);  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  outportb(0xA1, 0x02);  // ICW3: tell Slave PIC its cascade identity (0000 0010)

  outportb(0x21, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  outportb(0xA1, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
}

static
void set_irq_mask(uint64_t irq_mask)
{
  outportb(0x21, irq_mask & 0xff);
  outportb(0xA1, irq_mask >> 8);
}

static
void register_handler(uint16_t /*interrupt_number*/, func_t /*func*/)
{
}

// https://wiki.osdev.org/8259_PIC

static
interrupt_controller_vtable_t intel_8259_vtable =
{
  .setup_tables       = setup_tables,
  .remap_irqs         = remap_irqs,
  .set_irq_mask       = set_irq_mask,
  .register_handler   = register_handler,
  .enable             = enable,
  .disable            = disable,
};

static
driver_t intel_8259_driver = 
{
  .type    = driver_class::INTERRUPT_CONTROLLER,
  .id      = 0x12024, // TODO: how to assign these? in the register?
  .name    = { "intel_8259_pic" },
  .next    = nullptr,
  .prev    = nullptr,
  .load    = load,
  .unload  = unload,
  .vtable  = &intel_8259_vtable,
};

void register_intel_8259_driver()
{
  register_driver(intel_8259_driver);
}
