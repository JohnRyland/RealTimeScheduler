/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "types/integers.h"

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
uint16_t inportw(uint16_t port)
{
  uint16_t ret;
  asm volatile ( "inw %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline
void outportw(uint16_t port, uint16_t val)
{
  asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

static inline
uint32_t inportd(uint16_t port)
{
  uint32_t ret;
  asm volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline
void outportd(uint16_t port, uint32_t val)
{
  asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

static inline
void io_wait()
{
  outportb(0x80, 0x00);
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
uint64_t read_msr(uint32_t msr)
{
  uint64_t value;
  asm volatile ( "rdmsr" : "=A" (value) : "c" (msr) );
  return value;
}

static inline
void write_msr(uint32_t msr, uint64_t value)
{
  asm volatile ( "wrmsr" : : "c" (msr), "A" (value) );
}

static inline
void cpuid(int code, uint32_t* a, uint32_t* d)
{
  asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
}

static inline
uint64_t rdtsc()
{
  uint64_t ret;
  asm volatile ( "rdtsc" : "=A"(ret) );
  return ret;
}

static inline
void enable()
{
  asm volatile ( "sti" );
}

static inline
void disable()
{
  asm volatile ( "cli" );
}


#if 0

// https://wiki.osdev.org/Inline_Assembly/Examples

static inline
uint32_t farpeekl(uint16_t sel, void* off)
{
  uint32_t ret;
  asm ( "push %%fs\n\t"
      "mov  %1, %%fs\n\t"
      "mov  %%fs:(%2), %0\n\t"
      "pop  %%fs"
      : "=r"(ret) : "g"(sel), "r"(off) );
  return ret;
}

static inline
void farpokeb(uint16_t sel, void* off, uint8_t v)
{
  asm ( "push %%fs\n\t"
      "mov  %0, %%fs\n\t"
      "movb %2, %%fs:(%1)\n\t"
      "pop %%fs"
      : : "g"(sel), "r"(off), "r"(v) );
}

static inline
bool are_interrupts_enabled()
{
  unsigned long flags;
  asm volatile ( "pushf\n\t"
      "pop %0"
      : "=g"(flags) );
  return flags & (1 << 9);
}

static inline
unsigned long save_irqdisable()
{
  unsigned long flags;
  asm volatile ("pushf\n\tcli\n\tpop %0" : "=r"(flags) : : "memory");
  return flags;
}

static inline
void irqrestore(unsigned long flags)
{
  asm ("push %0\n\tpopf" : : "rm"(flags) : "memory","cc");
}

static inline
void lidt(void* base, uint16_t size)
{
  // This function works in 32 and 64bit mode
  struct {
    uint16_t length;
    void*    base;
  } __attribute__((packed)) IDTR = { size, base };
  asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

static inline
void invlpg(void* m)
{
  asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

#define fence() __asm__ volatile ("":::"memory")

#endif

