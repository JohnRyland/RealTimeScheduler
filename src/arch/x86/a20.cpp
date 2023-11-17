/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "arch/x86/constants.h"
#include "arch/x86/intrinsics.h"


// We have an asm equivalent for this in start.S
bool a20_is_enabled()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
  volatile uint8_t* addr0 = (uint8_t*)(0x00000500); // hopefully isn't an address that matters
  volatile uint8_t* addr1 = (uint8_t*)(0x00100500); // addr0 + 1MB  (if not A20 enabled, memory access will wrap)
  uint8_t orig_val0 = *addr0;
  uint8_t orig_val1 = *addr1;
  *addr0 = 0;
  *addr1 = 1;
  bool enabled = (*addr0 != *addr1);
  *addr0 = orig_val0;
  *addr1 = orig_val1;
  return enabled;
#pragma GCC diagnostic pop
}

void ppi_send_cmd(uint8_t cmd)
{
  while (!(inportb(PPI_STATUS) & 0x02)) /* wait */ ;
  outportb(PPI_COMMAND, cmd);
}

uint8_t ppi_read_data()
{
  while (!(inportb(PPI_STATUS) & 0x01)) /* wait */ ;
  return inportb(PPI_DATA);
}

void ppi_write_data(uint8_t val)
{
  while (!(inportb(PPI_STATUS) & 0x02)) /* wait */ ;
  outportb(PPI_DATA, val);
}

void ppi_enable_a20()
{
  disable(); // cli
  ppi_send_cmd(0xAD);
  ppi_send_cmd(0xD0);
  int data = ppi_read_data();
  ppi_send_cmd(0xD1);
  ppi_write_data(data | 0x02);
  ppi_send_cmd(0xAE);
  while (!(inportb(PPI_STATUS) & 0x02)) /* wait */ ;
  enable();  // sti
}

void fast_gate_enable_a20()
{
  uint8_t data = inportb(0x92);
  if (data & 0x2)
    return;
  outportb(0x92, (data | 0x02) & 0xFE);
}

/*

// We can't call 'int 15h' here from PM 32-bit as this
// is 16-bit BIOS code, we would need to use v86mode.
// But we do this in start.S already when still in 16-bit
// mode, so we have attempted this (but without the query
// if it is supported or it worked).
// We perhaps can skip trying that here, and attempt the
// other methods if we in fact didn't enable A20 from
// start.S.

bool query_a20_support()
{
  __asm__ __volatile__(
    "clc \n"
    "movw $0x2403,%%ax \n"
    "int  $0x15 \n"
    "setc %%al \n"
  );
  // TODO
  // if (al) false, else true
}

void bios_enable_a20()
{
  __asm__ __volatile__(
    "movw $0x2401,%%ax \n"
    "int  $0x15 \n"
    "setc %%al \n"
  );
  // TODO:
  // if (al) failed
}
*/

void enable_a20()
{
  disable(); // cli
//  enable();  // sti

  if (a20_is_enabled())
    return;
/*
  if (query_a20_support())
    bios_enable_a20();
  if (a20_is_enabled())
    return;
*/
  fast_gate_enable_a20();
  if (a20_is_enabled())
    return;
  ppi_enable_a20();
  if (!a20_is_enabled())
  {} // failed
}

