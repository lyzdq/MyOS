CC = gcc
CFLAGS = -m32 -fno-pie -ffreestanding -nostdlib -c -O0 -Wall
LD = ld
LDFLAGS = -m elf_i386 -T link.ld

all: os.img

boot32.bin: boot32.asm gdt.asm
	nasm -f bin boot32.asm -o boot32.bin

irq.o: irq.asm
	nasm -f elf32 irq.asm -o irq.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

kernel.bin: kernel.o irq.o link.ld
	$(LD) $(LDFLAGS) kernel.o irq.o -o kernel.bin

os.img: boot32.bin kernel.bin
	cat boot32.bin kernel.bin > os.img
	dd if=/dev/zero bs=512 count=56 >> os.img 2>/dev/null || true

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

clean:
	rm -f *.bin *.o *.img
