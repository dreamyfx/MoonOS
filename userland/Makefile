# Copyright (c) 2025-2026 Andrew (dreamyfx)
# MoonOS 1.0.0 x86_64 2026
# This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
# This header should be maintained in any file it is present in, as per the AGPL license terms.

CC    = x86_64-elf-gcc
NASM  = nasm
LD    = x86_64-elf-ld

CFLAGS = -O2 -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic \
         -m64 -mno-red-zone -Ilibc -Iarch

LDFLAGS = -T linker.ld -nostdlib

LIBC_OBJ = libc/stdlib.o
ASM_OBJ  = arch/crt0.o arch/syscall_asm.o

BINS = bin/msh.elf bin/uname.elf bin/halt.elf bin/reboot.elf

all: $(BINS)

libc/stdlib.o: libc/stdlib.c
	$(CC) $(CFLAGS) -c $< -o $@

arch/crt0.o: arch/crt0.asm
	$(NASM) -f elf64 $< -o $@

arch/syscall_asm.o: arch/syscall_asm.asm
	$(NASM) -f elf64 $< -o $@

msh/msh.o: msh/msh.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/uname.o: bin/uname.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/halt.o: bin/halt.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/reboot.o: bin/reboot.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/msh.elf: arch/crt0.o arch/syscall_asm.o msh/msh.o libc/stdlib.o
	$(LD) $(LDFLAGS) -o $@ $^

bin/uname.elf: arch/crt0.o arch/syscall_asm.o bin/uname.o libc/stdlib.o
	$(LD) $(LDFLAGS) -o $@ $^

bin/halt.elf: arch/crt0.o arch/syscall_asm.o bin/halt.o libc/stdlib.o
	$(LD) $(LDFLAGS) -o $@ $^

bin/reboot.elf: arch/crt0.o arch/syscall_asm.o bin/reboot.o libc/stdlib.o
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f libc/*.o arch/*.o msh/*.o bin/*.o $(BINS)
