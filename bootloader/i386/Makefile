


boot.bin: start.o
	# dd if=start.o   of=boot.bin bs=1 skip=32256
	# dd if=start.o   of=boot.bin bs=1 skip=0xBAFE
	dd if=start.o   of=boot.bin bs=1 skip=0xBAA0
	# dd if=start.o   of=boot.bin bs=1 skip=0x3AF1
	# dd if=/dev/zero of=boot.bin bs=512 seek=2 count=64


start.o: start.S
	# gcc -m32 -static -nostdlib -c main.c -o main.o
	gcc -m32 -static -nostdlib start.S main.c -o start.o -Wl,-U,start
	strip -x -S start.o

clean:
	rm start.o boot.bin main.o

dump: boot.bin
	hexdump -C boot.bin

test: boot.bin
	qemu-system-i386 -hda boot.bin

