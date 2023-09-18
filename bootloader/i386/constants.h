/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

// Defines and constants
// - Memory addresses
//   - Video
#        define   VGA_TEXT_BASE       0xB8000
#        define   VGA_TEXT_SIZE       0x8000
//   - Text attribute values
#        define   BOLD                0x0F
#        define   NORMAL              0x08
#        define   RED                 0x0C
#        define   GREEN               0x0A
// - Port addresses
//   - Programmable Interrupt Controller (8259)
#        define   PIC_MASTER_COMMAND  0x20
#        define   PIC_MASTER_DATA     0x21
#        define   PIC_SLAVE_COMMAND   0xA0
#        define   PIC_SLAVE_DATA      0xA1
//   - Programmable Interval Timer (8253/8254)
#        define   PIT_COMMAND         0x43
#        define   PIT_CHANNEL_0       0x40
#        define   PIT_CHANNEL_1       0x41
#        define   PIT_CHANNEL_2       0x42
//   - Programmable Peripheral Interface (8042)
#        define   PPI_DATA            0x60
#        define   PPI_COMMAND         0x64
#        define   PPI_STATUS          0x64

