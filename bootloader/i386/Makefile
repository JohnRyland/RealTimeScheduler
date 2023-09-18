#
# x86 OS Bootloader Makefile
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

ARCH=x86_64

# FMT=Elf
FMT=MachO

INCLUDES=-I../../include -I../../include/driver -I.
CFLAGS0=-Wall -static -ffreestanding -nostdlib $(INCLUDES)

# If using GNU Toolchain
ifeq ($(FMT),Elf)
  CC=$(ARCH)-elf-gcc
  LD=$(ARCH)-elf-ld
  CFLAGS=$(CFLAGS0)
  LFLAGS=-static -nostdlib -Ttext 0x0000 # -Tdata 0x10000 -Tbss 0x20000

# Else if using LLVM Toolchain on macOS
else ifeq ($(FMT),MachO)
  CC=gcc
  LD=ld
  CFLAGS=$(CFLAGS0) -m32 -fno-stack-protector -fno-builtin # -disable-simplify-libcalls
  LFLAGS=-static
endif

OBJCOPY=/usr/local/opt/binutils/bin/gobjcopy
SKIP=0x7C00

OBJS=start.o entry.o main.o task.o helpers.o schedule.o driver.o runtime.o

boot.bin: $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o boot.o
	$(LD) $(LFLAGS) -r $(OBJS) -o boot.o
	$(OBJCOPY) -O binary -x -S --set-start 0x7C00 --set-section-flags .bss=alloc,load,contents boot.o boot.obj
	dd if=boot.obj  of=boot.bin bs=1 skip=$(SKIP)
	dd if=/dev/zero of=boot.bin bs=512 seek=127 count=1

task.o: ../../src/task.cpp
main.o: ../../src/main.cpp
helpers.o: ../../src/helpers.cpp
schedule.o: ../../src/schedule.cpp

# driver.o: driver.cpp

%.o: ../../src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -std=c++11

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -std=c++11

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS) boot.o boot.obj boot.bin

dump: boot.bin
	hexdump -C boot.bin

test: boot.bin
	qemu-system-$(ARCH) boot.bin

