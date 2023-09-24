/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "stdint.h"

static inline
uint8_t inportb(uint16_t port)
{
  uint8_t ret;
  asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline
void outportb(uint16_t port, uint8_t val)
{
  asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

static inline
unsigned long read_cr0()
{
  unsigned long val;
  asm volatile ( "mov %%cr0, %0" : "=r"(val) );
  return val;
}

static inline
void write_cr0(uint32_t val)
{
  asm volatile ( "mov %0, %%cr0" : : "a"(val) : "memory" );
}

static inline
uint32_t read_cr3()
{
  uint32_t val;
  asm volatile ( "mov %%cr3, %0" : "=r"(val) );
  return val;
}

static inline
void write_cr3(uint32_t val)
{
  asm volatile ( "mov %0, %%cr3" : : "a"(val) : "memory" );
}

static inline
void enable()
{
  __asm__ __volatile__ ("sti");
}

static inline
void disable()
{
  __asm__ __volatile__ ("cli");
}

