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

# # If building from the i386 directory
I386_DIR := $(realpath .)
# ROOT_DIR = ../..
# # If building from the root directory
# I386_DIR = $(CURRENT_DIR)/bootloader/i386

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

# ABS_SOURCES := $(wildcard $(I386_DIR)/src/*.S $(I386_DIR)/src/*.cpp $(I386_DIR)/../../src/*.cpp)
# ABS_SOURCES  = $(I386_DIR)/src/start.S $(wildcard $(I386_DIR)/src/*.cpp )
ABS_SOURCES := $(wildcard $(I386_DIR)/src/*.cpp $(I386_DIR)/../../src/*.cpp)

# SOURCES      = $(ABS_SOURCES:$(I386_DIR)/%=%)) 
# SOURCES      = src/start.S $(ABS_SOURCES:$(I386_DIR)/%=%)) 
SOURCES      = src/start.S src/interrupts.S boot_sectors/volume_boot_record.S boot_sectors/master_boot_record.S $(ABS_SOURCES:$(I386_DIR)/%=%)) 


# ABS_INCLUDES = $(I386_DIR) $(I386_DIR)/include $(ROOT_DIR)/include $(ROOT_DIR)/include/driver $(ROOT_DIR)/include/runtime
# INCLUDES = $(I386_DIR) $(I386_DIR)/include $(ROOT_DIR)/include $(ROOT_DIR)/include/driver $(ROOT_DIR)/include/runtime
INCLUDES = . include ../../include ../../include/driver ../../include/runtime


CFLAGS   = $(FLAGS) -m32
CXXFLAGS = -std=c++11


bin/mbr.bin: $(OBJS_DIR)/boot_sectors/master_boot_record.S.o
	@mkdir -p bin
	$(LD) -static $(LDFLGS) $^ -o .build/mbr.o
	$(LD) -static $(LDFLG2) $^ -o .build/mbr.o
	$(OBJCOPY) -O binary -x -S --set-start $(START) .build/mbr.o .build/mbr.obj --image-base $(BASE)
	dd if=.build/mbr.obj of=$@ bs=1 skip=$(START)

bin/vbr.bin: $(OBJS_DIR)/boot_sectors/volume_boot_record.S.o
	@mkdir -p bin
	$(LD) -static $(LDFLGS) $^ -o .build/vbr.o
	$(LD) -static $(LDFLG2) $^ -o .build/vbr.o
	$(OBJCOPY) -O binary -x -S --set-start $(START) .build/vbr.o .build/vbr.obj --image-base $(BASE)
	dd if=.build/vbr.obj of=$@ bs=1 skip=$(START)

bin/kernel.bin: $(OBJS_DIR)/src/start.S.o $(OBJS_DIR)/src/interrupts.S.o $(OBJECTS)
	@mkdir -p bin
	$(LD) -static $(LDFLGS) $^ -o .build/kernel.o   # Without -r we check it doesn't have unresolved symbols
	$(LD) -static $(LDFLG2) $^ -o .build/kernel.o   # Potentially with -r we produce an object file instead of a binary
	$(OBJCOPY) --only-keep-debug --set-start 0x0000 --image-base 0x0000 .build/kernel.o bin/kernel.sym
	# $(OBJCOPY) -O binary -x -S --set-start $(START) --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents .build/kernel.o .build/kernel.obj # --pad-to $(END) --image-base $(BASE)
	# $(OBJCOPY) -O binary -x -S --set-start 0x7E00 --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents .build/kernel.o .build/kernel.obj --image-base 0x7E00 --pad-to 0x17E00
	$(OBJCOPY) -O binary --set-start 0x7E00 --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents .build/kernel.o .build/kernel.obj --image-base 0x7E00 --pad-to 0x17E00
	# dd if=.build/kernel.obj of=$@ bs=1 skip=$(START)
	dd if=.build/kernel.obj of=$@ bs=1 skip=0x7E00
	# dd if=/dev/zero of=bin/zeros bs=1 count=64k
	# cat .build/kernel.obj bin/zeros > $@

bin/image.bin: bin/mbr.bin bin/vbr.bin bin/kernel.bin
	@mkdir -p bin
	cat $^ > $@


# start.S.o must be first in the list of object files
bin/image2.bin: $(OBJS_DIR)/src/start.S.o $(OBJECTS)
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

bin/disk-image.qcow2: bin/image.bin
	qemu-img create -f qcow2 $@ 1M
	qemu-img dd -f raw -O qcow2 if=$< of=$@

do-4k-sector: bin/disk-image.qcow2
	# qemu-system-$(ARCH) -s -drive file=$<,format=raw,if=none,id=ID4096 -device ide-hd,drive=ID4096,physical_block_size=4096
	# qemu-system-$(ARCH) -s -drive file=$<,format=qcow2,physical_block_size=4096
	qemu-system-$(ARCH) -s -global ide-hd.physical_block_size=4096 -drive file=$<,format=qcow2

remote-debug: bin/image.bin
	killall -9 qemu-system-$(ARCH) || true
	qemu-system-$(ARCH) $< -s -S &

gdb:
	gdb -ex "set architecture i386:x86-64:intel" -ex "target remote localhost:1234" bin/image.sym


