
# Bootloaders
Copyright (C) 2023, John Ryland
All rights reserved.


## Required host tools to build and test:


### MacOS hosts
 - brew install qemu
 - brew install binutils
 - brew install gdb


## Possible target platforms:

Perhaps will implement in this order:
 - i386,    x86_64
 - arm,     aarch64
 - ppc,     ppc64
 - mips,    mips64
 - riscv32, riscv64
 - m68k
 - sh4
QEMU:
 - run from qemu - virtio drivers
Hosted:
 - run hosted build natively from macos, linux, windows
Development hosts:
 - develop from macos, linux, windows


## Current goals:

Need tools for editing the boot loader parameters.

The kernel needs some ability to do raw disk I/O in protected mode.

Need to add a networking driver.

It is not certain where in the kernel the disk code will be, so it
would be best not to rely on the kernel's disk code to load itself, but
instead have the bootloader do it.

Need to be able to switch back and forth between ring 0 and ring 3 - TSS/task gate,
or int 80h / syscall type interface, and the scheduler can run tasks in segregated
memory spaces between tasks. Need LDT - local desc tables perhaps. Some stuff to 
learn here.

Be able to log to serial / log file / console etc.  printk, log levels, syslog

system calls to give tasks ability to do stuff like I/O, network, memory.

Going wide:
  Multi-core support.

Going high:
  Long mode - 64-bit support.

Going Low:
  Porting to MCU - pi pico



## Done:

Debugging capabilities. Have a simple panic to print registers. Need to print
the call stack and be able to put symbols to that, so need table of address ranges
mapping to symbol names (file/line numbers too). Perhaps a tool to generate a
symbol map file. This needs a list of kernel address ranges which can be bsearched
for a matching entry which is a pointer to a string. Big string table of symbols.

So can currently chain load from MBR to VBR to kernel.

The loading of IDT is looking good.

Can now call the BIOS with PM -> realmode -> PM.

This allows screen mode setting so can set a VESA mode and get a linear address.

Have a serial driver which can print log to a serial console.

Can scan the PCIe bus.

The boot loader has parameters about the size of the kernel to
load so it can use int 13h to load a block of disk to a configured memory address.
Params like disk LBA start, sector count, and memory address. Also some
kernel are passed init params, the boot loader stores and passes
on to the kernel some params, like safe-mode, or recovery mode or such or the
root-fs to mount.

Currently the param is limited to 16 characters, but there
are 4 boot entries to choose from.

When building with elf, can connect a debugger inside VSCode.

