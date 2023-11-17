#
# x86 OS Bootloader Makefile
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

PROJECT   = x86-Diskimage
TARGET    = kernel.o
ARCH      = x86_64
HOST      = macOS

#  Option of the intermediate object format:  macho|elf
FMT       = macho
# FMT       = elf

#  Option of which toolchain is used:  gcc|clang
# TOOLCHAIN = gcc
TOOLCHAIN = clang

#
# Building on a macOS host system, build option support is as follows:
#
# ============================================
#  toolchain\fmt  ||    elf   |     macho
# ================++==========+===============
#  gcc            ||    Yes   | not-supported
# ----------------+-----------+---------------
#  clang          ||   Yes(*) |    partial
# --------------------------------------------
#
#  not-supported - setting macho+gcc will ignore using gcc and uses clang.
#
#  partial       - depends on the target, eg 'make micro' works,
#                  working now for others, but clang gets stuck on recursion and
#                  some other constructs. I think there might be additional compiler
#                  flags that need to be set, possibly related to the stack.
#
#  yes(*)        - It could be that clang is just deferring to gcc for
#                  the elf builds and it isn't actually clang that is building.
#



DBG_FLAGS = -g3 -ggdb
FLAGS    = -Wall -Wextra -Werror -static -nostdlib -fno-common -ffreestanding \
					 -mno-red-zone -fomit-frame-pointer -fno-stack-protector -fno-builtin \
					 -fno-PIC -fno-exceptions -fno-rtti -march=i586 -D$(FMT)_fmt
           # -no-pie -ffunction-sections -fdata-sections

# FLAGS    = -ffreestanding -fno-builtin -D$(FMT)_fmt

# # If building from the i386 directory
I386_DIR := $(realpath .)
# ROOT_DIR = ../..
# # If building from the root directory
# I386_DIR = $(CURRENT_DIR)/bootloader/i386

# If using GNU Toolchain
ifeq ($(FMT),elf)
  ifeq ($(TOOLCHAIN),gcc)
		CC     = $(ARCH)-elf-g++
		LD     = $(ARCH)-elf-ld
		FLAGS2 = $(FLAGS) -O0  # -O0 -g # -O5
		LDFLGS = -nostdlib -melf_i386 -Ttext 0x0000 # -Tdata 0xc000 -Tbss 0xd000
		LDFLG2 = $(LDFLGS)
  else ifeq ($(TOOLCHAIN),clang)
		CC     = g++
		LD     = $(ARCH)-elf-ld
		FLAGS2 = $(FLAGS) -O0 -target x86_64-apple-none-elf
		LDFLGS = -nostdlib -melf_i386 -Ttext 0x0000 # -Tdata 0xc000 -Tbss 0xd000
		LDFLG2 = $(LDFLGS)
	endif
# Else if using LLVM Toolchain on macOS
else ifeq ($(FMT),macho)
  CC     = g++
  LD     = ld
	FLAGS2 = $(FLAGS) -O1 -fno-split-stack -fno-stack-check
           # -flto -g -fpcc-struct-return -fno-relaxed-template-template-args
  LDFLGS = 
  LDFLG2 = -r
endif

CXX      = $(CC)
LINKER   = $(LD)
LFLAGS   = -static $(LDFLG2)
OBJCOPY  = /usr/local/opt/binutils/bin/gobjcopy

START        = 0x00007C00
BASE         = 0x00007C00
KERNEL_START = 0x00007E00
KERNEL_BASE  = 0x00007E00
KERNEL_END   = 0x00047E00  # 64Kb

CONFIG    = i386
OS_DEFINE = _I386

ABS_SOURCES := $(wildcard $(I386_DIR)/src/*.cpp $(I386_DIR)/../../src/*.cpp $(I386_DIR)/../../src/initialize/*.cpp $(I386_DIR)/../../src/arch/x86/*.cpp $(I386_DIR)/../../src/kernel/*.cpp $(I386_DIR)/../../src/runtime/*.cpp $(I386_DIR)/../../src/modules/*.cpp $(I386_DIR)/../../src/demo/*.cpp ) #  $(I386_DIR)/../../src/demo/main.cpp )

SOURCES  = src/start.S src/end.S src/interrupts.S boot_sectors/volume_boot_record.S boot_sectors/master_boot_record.S $(ABS_SOURCES:$(I386_DIR)/%=%)) 

INCLUDES = . include ../../configs/${CONFIG} ../../include ../../include/kernel ../../include/module ../../include/runtime ../../include/common ../../include/arch/x86

# start.S.o must be first in the list of object files and end.S.o last.
OBJECTS  := $(OBJS_DIR)/src/start.S.o $(OBJS_DIR)/src/interrupts.S.o $(filter-out %.S.o,$(OBJECTS)) $(OBJS_DIR)/src/end.S.o

BUILD_TYPE_CFLAGS = -DNDEBUG
CFLAGS   = $(FLAGS2) -m32 -D$(OS_DEFINE) $(DBG_FLAGS)
CXXFLAGS = -std=c++2b


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

# start.S.o must be first in the list of object files
.build/kernel.o: $(OBJECTS) #  $(OBJS_DIR)/src/start.S.o $(OBJS_DIR)/src/interrupts.S.o $(OBJECTS) $(OBJS_DIR)/src/end.S.o
	$(LD) -static $(LDFLGS) $^ -o $@   # Without -r we check it doesn't have unresolved symbols
	$(LD) -static $(LDFLG2) $^ -o $@   # Potentially with -r we produce an object file instead of a binary

# This is the target when you run 'make', but we add a dependancy to it which
# then basically builds everything for the image to be run. Just run 'make normal' or
# other target to run it.
bin/kernel.o: bin/image.qcow2

bin/kernel.sym: .build/kernel.o
	@mkdir -p bin
ifeq ($(FMT),macho)
	cp $^ $@
else
	$(OBJCOPY) --only-keep-debug $^ $@
endif

bin/kernel.obj: .build/kernel.o
	@mkdir -p bin
	# objcopy --rename-section .ctors=.init_array \
	#         --rename-section .dtors=.fini_array $file
	$(OBJCOPY) --set-start   $(KERNEL_START) \
		         --image-base  $(KERNEL_BASE)  \
						 --pad-to      $(KERNEL_END)   \
	           --set-section-flags .text=alloc,load,contents \
		         --set-section-flags .data=alloc,load,contents \
	           --set-section-flags .bss=alloc,load,contents  \
	           -O binary -x -S  $^ $@

bin/kernel.bin: bin/kernel.obj
	@mkdir -p bin
	dd if=$^ of=$@ bs=1 skip=$(KERNEL_START)

bin/symbol_map_gen: ../../tools/symbol_map/symbol_map_gen.cpp
	g++ $< -o $@ -std=c++11 -I../../include

bin/kernel.map: bin/kernel.sym bin/symbol_map_gen
	# nm -n $< | grep " [t|T] " | c++filt | ./bin/symbol_map_gen > $@
	nm -n $< | grep " T " | c++filt | ./bin/symbol_map_gen > $@

bin/image.bin: bin/mbr.bin bin/vbr.bin bin/kernel.bin bin/kernel.map
	@mkdir -p bin
	cat $^ > $@

dump: bin/image.bin
	hexdump -C $<

misc: bin/image.qcow2
	# -serial telnet:localhost:4321,server,nowait
	# qemu-system-$(ARCH) -serial msmouse -serial mon:stdio -cpu max $<
	# qemu-system-$(ARCH) -serial stdio -serial msmouse -cpu max $<
	# qemu-system-$(ARCH) -serial telnet:localhost:4321,server,nowait -serial msmouse -cpu max $<
	# qemu-system-$(ARCH) -serial telnet:localhost:4321,server,nowait -cpu max $<
	# qemu-system-$(ARCH) -serial telnet:localhost:4321,server,nowait --chardev msmouse,id=id -cpu max $<
	# qemu-system-$(ARCH) --chardev msmouse,id=id -cpu max $<
	# qemu-system-$(ARCH) -cpu max $<
	# qemu-system-i386 -serial msmouse -cpu max $<
	# qemu-system-$(ARCH) -vga cirrus -full-screen -serial stdio -cpu max -machine type=q35 $<

clear:
	clear

# Emulate a minimualist machine type
micro: bin/image.qcow2 clear
	qemu-system-$(ARCH) -M microvm -nographic -drive id=sda,file=$< -device virtio-blk-device,drive=sda

# Emulate an old early 1990s era ISA-PC
legacy: bin/image.qcow2 clear
	qemu-system-$(ARCH) -M isapc -vga std -serial stdio -cpu max $<

# Emulate a late 1990s, early 2000s era PCI PC with 2 cores
pc: bin/image.qcow2 clear
	qemu-system-$(ARCH) -M pc -smp 2 -usb -net nic,model=e1000 -vga std -serial stdio $<

# Outputs to VGA
vga: bin/image.qcow2
	qemu-system-$(ARCH) -device VGA,vgamem_mb=64 -full-screen -serial stdio -cpu max -machine type=q35 $<

# Outputs to both serial and to VGA
serial: bin/image.qcow2
	qemu-system-$(ARCH) -device VGA,vgamem_mb=64 -serial stdio -cpu max -machine type=q35 $<

# Outputs only to serial
headless: bin/image.qcow2
	qemu-system-$(ARCH) -vga none -nographic -cpu max -machine type=q35 $<

normal: bin/image.qcow2 clear
	qemu-system-$(ARCH) -M q35 -usb -net nic,model=rtl8139 -device virtio-gpu -serial stdio -cpu max $<

# Emulate a more modern PC with 4-cores, multiple PCIe buses
modern: bin/image.qcow2 clear
	qemu-system-$(ARCH) -smp cores=4 -usb -m 8G -serial stdio -machine type=q35 $< \
    -device pcie-root-port,bus=pcie.0,id=pci.1,slot=1 -device VGA,vgamem_mb=64,bus=pci.1 \
    -device pcie-root-port,bus=pcie.0,id=pci.2,slot=2 -device e1000,bus=pci.2

# boot entry 0 opts: "               "
# boot entry 1 opts: "no_vga         "
# boot entry 2 opts: "quiet no_vga   "
# boot entry 3 opts: "no_modules     "

# Switch the default boot menu item in the bootloader
x00: bin/image.bin
	printf '\$@' | dd of=$< bs=1 seek=516 count=1 conv=notrunc
x01: bin/image.bin
	printf '\$@' | dd of=$< bs=1 seek=516 count=1 conv=notrunc
x02: bin/image.bin
	printf '\$@' | dd of=$< bs=1 seek=516 count=1 conv=notrunc
x03: bin/image.bin
	printf '\$@' | dd of=$< bs=1 seek=516 count=1 conv=notrunc

# Boot in to a particular boot menu item
boot-0: x00 normal
boot-1: x01 normal
boot-2: x02 headless
boot-3: x03 normal


# Starts with waiting for debugger to attach
vscode-debug: bin/image.qcow2
	killall -9 qemu-system-$(ARCH) || true
	# qemu-system-$(ARCH) -vga cirrus -full-screen -serial stdio -cpu max -machine type=q35 $< -s -S &
	qemu-system-$(ARCH) -usb -device usb-ehci,id=ehci -vga virtio -full-screen -serial stdio -cpu max -machine type=q35 $< -s -S &
	# qemu-system-$(ARCH) -device VGA,bus=pcie.0 -full-screen -serial stdio -cpu max -machine type=q35 $< -s -S &

# Kill any running qemu processes
stop:
	killall -9 qemu-system-$(ARCH) || true

bin/image.qcow2: stop bin/image.bin
	qemu-img convert -O qcow2 bin/image.bin $@
	qemu-img resize $@ 5M


# Attempted test for simulating HDD with 4KB per sector instead of the normal 512 byte sector size
do-4k-sector: bin/image.qcow2
	# qemu-system-$(ARCH) -s -drive file=$<,format=raw,if=none,id=ID4096 -device ide-hd,drive=ID4096,physical_block_size=4096
	# qemu-system-$(ARCH) -s -drive file=$<,format=qcow2,physical_block_size=4096
	qemu-system-$(ARCH) -cpu max -s -global ide-hd.physical_block_size=4096 -drive file=$<,format=qcow2

# Similar to vscode-debug, runs waiting for a debugger to attach
remote-debug: bin/image.qcow2
	killall -9 qemu-system-$(ARCH) || true
	qemu-system-$(ARCH) -serial stdio -cpu max -machine type=q35 $< -s -S &

# Runs a debugger and launchs 'make remote-debug' which it then attachs to and runs
gdb: bin/image.qcow2
	gdb -ex "shell make remote-debug" -ex "set architecture i386:x86-64:intel" -ex "target remote localhost:1234" -ex "cont" bin/kernel.sym


