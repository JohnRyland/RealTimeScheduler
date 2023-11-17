/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/
#pragma once

#include "driver.h"

enum class baud_rate_t : uint8_t
{
              // divisor for the 16650
  BAUD_300,   // 0x0180
  BAUD_600,   // 0x00C0
  BAUD_1200,  // 0x0060
  BAUD_2400,  // 0x0030
  BAUD_4800,  // 0x0018
  BAUD_9600,  // 0x000C
  BAUD_19200, // 0x0006
  BAUD_38400, // 0x0003
  BAUD_57600, // 0x0002
  BAUD_115200,// 0x0001
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

struct serial_driver_vtable_t
{
  bool (*initialize)(driver_t& driver, baud_rate_t baud, uint8_t bits, bool parity, bool stop);
  bool (*ready_to_recv)(driver_t& driver);
  bool (*ready_to_send)(driver_t& driver);
  uint8_t (*recv)(driver_t& driver);
  void (*send)(driver_t& driver, uint8_t data);
};
