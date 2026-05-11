# Copyright (c) 2025-2026 Andrew (dreamyfx)
# MoonOS 1.0.0 x86_64 2026
# This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
# This header should be maintained in any file it is present in, as per the AGPL license terms.

CC     = x86_64-elf-gcc
LD     = x86_64-elf-ld
NASM   = nasm
XORRISO = xorriso

BUILD  = build
ISO    = moonos.iso
KERNEL = $(BUILD)/moonos.elf

CFLAGS = -g -O2 -std=gnu11 -ffreestanding -fno-stack-protector -fno-lto \
         -fPIE -m64 -mno-red-zone -Wall -Wextra \
         -Ikernel -Ikernel/boot -Ikernel/lib -Ikernel/cpu \
         -Ikernel/mm -Ikernel/proc -Ikernel/syscall \
         -Ikernel/io -Ikernel/acpi -Ikernel/fs -Ikernel/tty

LDFLAGS = -m elf_x86_64 -nostdlib -static -pie --no-dynamic-linker \
          -z text -z max-page-size=0x1000 -T linker.ld

NASMFLAGS = -f elf64

LIMINE_VER = 10.8.2

K_C_SRCS = \
  kernel/boot/kmain.c \
  kernel/lib/kstring.c kernel/lib/console.c kernel/lib/panic.c \
  kernel/cpu/gdt.c kernel/cpu/idt.c kernel/cpu/lapic.c \
  kernel/mm/pmm.c kernel/mm/vmm.c kernel/mm/heap.c \
  kernel/proc/process.c kernel/proc/elf.c \
  kernel/syscall/syscall.c \
  kernel/io/ps2.c kernel/io/serial.c kernel/io/rtc.c \
  kernel/fs/vfs.c kernel/fs/tarfs.c kernel/fs/procfs.c \
  kernel/tty/tty.c

K_ASM_SRCS = \
  kernel/arch/boot.asm \
  kernel/arch/gdt_asm.asm \
  kernel/arch/interrupts.asm \
  kernel/arch/syscall_entry.asm \
  kernel/arch/process_asm.asm

K_C_OBJS   = $(patsubst %.c,  $(BUILD)/%.o, $(K_C_SRCS))
K_ASM_OBJS = $(patsubst %.asm,$(BUILD)/%.o, $(K_ASM_SRCS))
OBJ_ALL    = $(K_C_OBJS) $(K_ASM_OBJS)

.PHONY: all clean run limine-setup

all: $(ISO)

$(BUILD)/%.o: %.c | $(BUILD)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm | $(BUILD)
	@mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) $< -o $@

$(KERNEL): $(OBJ_ALL)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $@"
	$(MAKE) -C userland
	@echo "Userland built."

$(BUILD)/initrd.tar: $(KERNEL)
	@rm -rf $(BUILD)/initrd
	@mkdir -p $(BUILD)/initrd/bin
	@cp userland/bin/*.elf $(BUILD)/initrd/bin/ 2>/dev/null || true
	@cd $(BUILD)/initrd && COPYFILE_DISABLE=1 tar --exclude="._*" -cf ../initrd.tar *
	@echo "Initrd created."

$(ISO): $(KERNEL) $(BUILD)/initrd.tar limine.conf limine-setup
	@rm -rf iso_root
	@mkdir -p iso_root/EFI/BOOT
	@cp $(KERNEL) iso_root/moonos.elf
	@cp limine.conf iso_root/
	@printf "    module_path: boot():/initrd.tar\n" >> iso_root/limine.conf
	@cp $(BUILD)/initrd.tar iso_root/
	@cp background.png iso_root/ 2>/dev/null || true
	@cp limine/limine-bios.sys iso_root/
	@cp limine/limine-bios-cd.bin iso_root/
	@cp limine/limine-uefi-cd.bin iso_root/
	@cp limine/BOOTX64.EFI iso_root/EFI/BOOT/
	@cp limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	$(XORRISO) -as mkisofs -R -J -b limine-bios-cd.bin \
	  -no-emul-boot -boot-load-size 4 -boot-info-table \
	  --efi-boot limine-uefi-cd.bin \
	  -efi-boot-part --efi-boot-image --protective-msdos-label \
	  iso_root -o $(ISO)
	./limine/limine bios-install $(ISO)
	@echo "ISO ready: $(ISO)"

limine-setup:
	@if [ ! -f limine/limine-bios.sys ]; then \
	  git clone https://github.com/limine-bootloader/limine.git \
	    --branch=v$(LIMINE_VER)-binary --depth=1 limine; \
	fi
	@if [ ! -f kernel/boot/limine.h ]; then \
	  cp limine/limine.h kernel/boot/limine.h 2>/dev/null || true; \
	fi
	$(MAKE) -C limine

$(BUILD):
	mkdir -p $(BUILD)

run: $(ISO)
	qemu-system-x86_64 -m 4G -serial stdio -cdrom $(ISO) -boot d \
	  -smp 2 -vga std \
	  -global VGA.xres=1920 -global VGA.yres=1080

clean:
	rm -rf $(BUILD) iso_root $(ISO)
	$(MAKE) -C userland clean
