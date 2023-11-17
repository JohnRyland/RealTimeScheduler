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

// - Port addresses
//   - Programmable Peripheral Interface  (8042)
#        define   PPI_DATA            0x60
#        define   PPI_COMMAND         0x64
#        define   PPI_STATUS          0x64
//   - DMA Controller                     (8237)
//                                    0x00->0x1F
//   - CMOS RTC                           (????)
//                                    0x70->0x71
//   - UART                               (8250)
#        define   COM_BUFFER          0x3F8
#        define   COM_IER             0x3F9
#        define   COM_IIR             0x3FA
#        define   COM_LCR             0x3FB
#        define   COM_MCR             0x3FC
#        define   COM_LSR             0x3FD
#        define   COM_MSR             0x3FE
#        define   COM_PAD             0x3FF
//   - Programmable Interval Timer        (8253/8254)
#        define   PIT_COMMAND         0x43
#        define   PIT_CHANNEL_0       0x40
#        define   PIT_CHANNEL_1       0x41
#        define   PIT_CHANNEL_2       0x42
//   - Programmable Interrupt Controller  (8259)
#        define   PIC_MASTER_COMMAND  0x20
#        define   PIC_MASTER_DATA     0x21
//                                    0x30 -> 0x3F for slave also 
#        define   PIC_SLAVE_COMMAND   0xA0
#        define   PIC_SLAVE_DATA      0xA1
//   - Floppy Controller                  (8272)
#        define   FDC_DISK_STATUS_A   0x3F0
#        define   FDC_DISK_STATUS_B   0x3F1
#        define   FDC_OUTPUT          0x3F2
#        define   FDC_MAIN_STATUS     0x3F4
#        define   FDC_COMMAND_STATUS  0x3F5
#        define   FDC_INPUT           0x3F7
// Ports 3B0-3BB, 3C0-3CB are MCGA and CGA (6845)
// Port 0x201 is game port


