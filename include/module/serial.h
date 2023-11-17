/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "../types.h"

enum class baud_rate_t : uint8_t
{
  BAUD_300,
  BAUD_600,
  BAUD_1200,
  BAUD_2400,
  BAUD_4800,
  BAUD_9600,
  BAUD_19200,
  BAUD_38400,
  BAUD_57600,
  BAUD_115200,
};

enum class word_size_t : uint8_t
{
  WORD_SIZE_5_BITS,
  WORD_SIZE_6_BITS,
  WORD_SIZE_7_BITS,
  WORD_SIZE_8_BITS,
};

enum class stop_bits_t : uint8_t
{
  ONE_STOP_BIT,
  TWO_STOP_BITS,  // This happens to be 1.5 bits in the case where the word size is set to 5 bits
};

enum class parity_t : uint8_t
{
  NO_PARITY,
  ODD_PARITY,
  EVEN_PARITY,
  MARK_PARITY,    // For testing the remote is checking parity or gaining an extra stop bit
  SPACE_PARITY,   // For testing the remote is checking parity
};

struct serial_driver_t;

struct serial_driver_vtable_t
{
  void (*initialize)(serial_driver_t* driver, baud_rate_t baud, word_size_t bits, stop_bits_t stop, parity_t parity);
  bool (*ready_to_recv)(serial_driver_t* driver);
  bool (*ready_to_send)(serial_driver_t* driver);
  void (*irq_service_handler)(serial_driver_t* driver);
  uint8_t (*recv)(serial_driver_t* driver);
  void (*send)(serial_driver_t* driver, uint8_t data);
};
