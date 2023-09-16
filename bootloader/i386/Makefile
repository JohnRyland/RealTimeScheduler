#
# x86 OS Bootloader Makefile
# Copyright (C) 2023, John Ryland
# All rights reserved.
#

boot.bin: start.o main.o
	ld -static -no_pie -r start.o main.o -o boot.o -U start
	strip -x -S boot.o
	dd if=boot.o    of=boot.bin bs=1 skip=0x7DCC
	dd if=/dev/zero of=boot.bin bs=512 seek=6 count=64

main.o: main.c
	gcc -m32 -static -nostdlib -c main.c -o main.o

start.o: start.S main.c
	gcc -m32 -static -nostdlib -c start.S -o start.o

clean:
	rm start.o boot.bin main.o boot.o

dump: boot.bin
	hexdump -C boot.bin

test: boot.bin
	qemu-system-i386 -hda boot.bin

