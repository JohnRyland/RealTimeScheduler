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

// OCW1 - Operation Control Word 1

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
uint64_t get_irq_mask()
{
  return inportb(0x21) | (inportb(0xA1) << 8);
}

static
const char* processor_fault_description(uint8_t processor_fault_number)
{
  switch (processor_fault_number)
  {
    case 0x00: return "Division by zero";
    case 0x01: return "Single-step trap";
    case 0x02: return "Non-maskable interrupt";
    case 0x03: return "Debug breakpoint";
    case 0x04: return "Arithmetic overflow";
    case 0x05: return "Bounds range exceeded";
    case 0x06: return "Invalid opcode";
    case 0x07: return "Coprocessor not available";
    case 0x08: return "Double fault";
    case 0x09: return "Coprocessor segment overrun";
    case 0x0A: return "Invalid task state segment";
    case 0x0B: return "Segment not present";
    case 0x0C: return "Stack segment fault";
    case 0x0D: return "General protection fault";
    case 0x0E: return "Page fault";
    case 0x0F: return "Reserved";
    case 0x10: return "x87 floating point exception";
    case 0x11: return "Alignment check";
    case 0x12: return "Machine check";
    case 0x13: return "SIMD floating point exception";
    case 0x14: return "Virtualization exception";
    case 0x15: return "Control protection exception";
    case 0x16: return "Reserved (0x16)";
    case 0x17: return "Reserved (0x17)";
    case 0x18: return "Reserved (0x18)";
    case 0x19: return "Reserved (0x19)";
    case 0x1A: return "Reserved (0x1A)";
    case 0x1B: return "Reserved (0x1B)";
    case 0x1C: return "Reserved (0x1C)";
    case 0x1D: return "Reserved (0x1D)";
    case 0x1E: return "Reserved (0x1E)";
    case 0x1F: return "Reserved (0x1F)";
  }
  return "Undefined";
}

static
const char* hardware_interrupt_description(uint8_t hardware_interrupt_number)
{
  switch (hardware_interrupt_number)
  {
    case 0x20: return "PIT / System timer";
    case 0x21: return "Keyboard";
    case 0x22: return "8259A slave controller / PIC";
    case 0x23: return "COM2 / COM4";
    case 0x24: return "COM1 / COM3";
    case 0x25: return "Sound card / LPT2";
    case 0x26: return "Floppy disk controller";
    case 0x27: return "Parallel port (LPT1)";
    case 0x28: return "Real time clock (RTC)";
    case 0x29: return "Redirected master IRQ2";
    case 0x2A: return "PCI devices 1";
    case 0x2B: return "PCI devices 2";
    case 0x2C: return "PS/2 Mouse controller";
    case 0x2D: return "Math Coprocessor";
    case 0x2E: return "Hard disk controller 1";
    case 0x2F: return "Hard disk controller 2";
  }
  return "Undefined";
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
