

all: decode-1.txt decode-2.txt


# Parse using the dtc command
decode-1.txt: qemu.dtb
	dtc -I dtb $< > $@


# Parse with our own parser which attempts to reproduce the same/similar output
decode-2.txt: qemu.dtb device-tree-parser
	./device-tree-parser $< > $@


qemu.dtb:
	qemu-system-arm -machine virt -machine dumpdtb=$@


device-tree-parser: device-tree-parser.cpp
	$(CC) -O0 $< -o $@


clean:
	rm qemu.dtb decode-1.txt decode-2.txt device-tree-parser

