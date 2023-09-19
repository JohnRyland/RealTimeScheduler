#
# x86 OS Bootloader Makefile
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

PROJECT  = x86-Diskimage
TARGET   = boot.o
ARCH     = x86_64
#FMT      = Elf
FMT      = MachO
FLAGS    = -Wall -static -nostdlib -ffreestanding -fno-stack-protector -fno-builtin

# If using GNU Toolchain
ifeq ($(FMT),Elf)

  CC     = $(ARCH)-elf-gcc
  LD     = $(ARCH)-elf-ld
  LFLAGS = $(FLAGS) -Wl,-melf_i386
  LDFLGS = -static -nostdlib -melf_i386 -Ttext 0x0000 -Tdata 0x10000 -Tbss 0x20000

# Else if using LLVM Toolchain on macOS
else ifeq ($(FMT),MachO)

  CC     = gcc
  LD     = ld
  LFLAGS = $(FLAGS) -m32

endif

CXX      = $(CC)
LINKER   = $(CC)
OBJCOPY  = /usr/local/opt/binutils/bin/gobjcopy
START    = 0x7C00
SOURCES  = $(wildcard src/*.S src/*.cpp ../../src/*.cpp)
INCLUDES = include ../../include ../../include/driver .
CFLAGS   = $(FLAGS) -m32
CXXFLAGS = -std=c++11

# start.S.o must be first in the list of object files
bin/image.bin: .build/Darwin-release/objs/src/start.S.o $(OBJECTS)
	@mkdir -p bin
	$(LD) -static $(LDFLGS) $^ -o .build/boot.o
	$(LD) -static $(LDFLGS) -r $^ -o .build/boot.o
	$(OBJCOPY) -O binary -x -S --set-start $(START) --set-section-flags .bss=alloc,load,contents .build/boot.o .build/boot.obj
	dd if=.build/boot.obj of=$@ bs=1 skip=$(START)
	dd if=/dev/zero of=$@ bs=512 seek=64 count=1

dump: bin/image.bin
	hexdump -C $<

do: bin/image.bin
	qemu-system-$(ARCH) $<

