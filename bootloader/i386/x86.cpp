/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "x86.h"

int inportb(int port)
{
  int val = 0;
  __asm__ __volatile__ (
    "movl %%eax,%%eax\n"
    "movl %1,%%edx\n"
    "inb  %%dx,%%al  # read the port\n"
    "movl %%eax,%0"
    : "=r"(val)
    : "r"(port)
    : "%eax", "%edx");
  return val;
}

void outportb(int port, int val)
{
  __asm__ __volatile__ (
    "xorl %0,%%eax\n"
    "movl %1,%%edx\n"
    "outb %%al,%%dx  # write to the port\n"
    : : "r"(val), "r"(port)
    : "%eax", "%edx");
}

void enable()
{
  __asm__ __volatile__ ("sti");
}

void disable()
{
  __asm__ __volatile__ ("cli");
}

isr_routine_t getvect(int vec)
{
  return nullptr;
}

void setvect(int vec, isr_routine_t)
{
}

