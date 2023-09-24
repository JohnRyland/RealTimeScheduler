#
# x86 OS Bootloader Makefile
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

PROJECT  = x86-Diskimage
TARGET   = boot.o
ARCH     = x86_64
# FMT      = Elf
FMT      = MachO
FLAGS    = -g -Wall -static -nostdlib -ffreestanding -fno-stack-protector -fno-builtin -fno-PIC -fno-exceptions -fno-rtti

# If using GNU Toolchain
ifeq ($(FMT),Elf)

  CC     = $(ARCH)-elf-gcc
  LD     = $(ARCH)-elf-ld
  LFLAGS = $(FLAGS) -Wl,-melf_i386
  LDFLGS = -nostdlib -melf_i386 -Ttext 0x0000 # -Tdata 0xc000 -Tbss 0xd000
  LDFLG2 = $(LDFLGS)

# Else if using LLVM Toolchain on macOS
else ifeq ($(FMT),MachO)

  CC     = gcc
  LD     = ld
  LFLAGS = $(FLAGS) -m32
  LDFLGS =
  LDFLG2 = -r

endif

CXX      = $(CC)
LINKER   = $(CC)
OBJCOPY  = /usr/local/opt/binutils/bin/gobjcopy
START    = 0x00007C00
BASE     = 0x00007C00
END      = 0x00017C00  # 64Kb
SOURCES  = $(wildcard src/*.S src/*.cpp ../../src/*.cpp)
INCLUDES = include ../../include ../../include/driver ../../include/runtime .
CFLAGS   = $(FLAGS) -m32
CXXFLAGS = -std=c++11


# start.S.o must be first in the list of object files
bin/image.bin: $(OBJS_DIR)/src/start.S.o $(OBJECTS)
	@mkdir -p bin
	$(LD) -static $(LDFLGS) $^ -o .build/image.o   # Without -r we check it doesn't have unresolved symbols
	$(LD) -static $(LDFLG2) $^ -o .build/image.o   # Potentially with -r we produce an object file instead of a binary
	$(OBJCOPY) --only-keep-debug --set-start 0x0000 --image-base 0x0000 .build/image.o bin/image.sym
	$(OBJCOPY) -O binary -x -S --set-start $(START) --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents .build/image.o .build/boot.obj --pad-to $(END) --image-base $(BASE)
	dd if=.build/boot.obj of=$@ bs=1 skip=$(START)

dump: bin/image.bin
	hexdump -C $<

do: bin/image.bin
	qemu-system-$(ARCH) -s $<

remote-debug: bin/image.bin
	killall -9 qemu-system-$(ARCH) || true
	qemu-system-$(ARCH) $< -s -S &

gdb:
	gdb -ex "set architecture i386:x86-64:intel" -ex "target remote localhost:1234" bin/image.sym


