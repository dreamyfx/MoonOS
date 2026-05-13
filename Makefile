# MoonOS Makefile
# Requirements: nasm, xorriso, gcc-x86-64-linux-gnu, binutils-x86-64-linux-gnu

CROSS   ?= x86_64-linux-gnu
CC       = $(CROSS)-gcc
AS       = nasm
LD       = $(CROSS)-ld
OBJCOPY  = $(CROSS)-objcopy

CFLAGS   = -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone \
           -mno-mmx -mno-sse -mno-sse2 \
           -mcmodel=kernel \
           -O2 -Wall -Wextra \
           -Isrc \
           -std=c11 \
           -DMOS_VERSION=\"$(VERSION)\" \
           -DMOS_CODENAME=\"$(CODENAME)\" \
           -DMOS_NAME=\"$(OS_NAME)\"

LDFLAGS  = -T src/linker.ld -nostdlib -z max-page-size=0x1000 \
           --no-dynamic-linker

ASFLAGS  = -f elf64

VERSION    := $(shell grep '^version=' moonos.txt | cut -d= -f2)
CODENAME   := $(shell grep '^codename=' moonos.txt | cut -d= -f2)
OS_NAME    := $(shell grep '^name=' moonos.txt | cut -d= -f2)

LIMINE_DIR ?= limine
ISO_MAIN   := dist/moonos.iso
ISO_VER    := dist/moonos$(VERSION).iso

KERNEL_SRCS = src/boot.asm          \
              src/kernel.c          \
              src/commands.c        \
              src/apps/system.c     \
              src/apps/fscmds.c     \
              src/apps/misc.c       \
              src/apps/snake.c      \
              src/fb.c              \
              src/font.c            \
              src/keyboard.c        \
              src/mouse.c           \
              src/gui.c             \
              src/util.c            \
              src/ramdisk.c         \
              src/arcpad.c          \
              src/ata.c             \
              src/pci.c             \
              src/progressbar.c

KERNEL_OBJS = $(patsubst src/%.asm, build/%.o, \
              $(patsubst src/%.c,   build/%.o, \
              $(patsubst src/apps/%.c, build/apps/%.o, \
              $(patsubst src/net/%.c,  build/net/%.o, $(KERNEL_SRCS)))))

.PHONY: all clean iso run

all: $(ISO_MAIN)

build:
	mkdir -p build build/apps build/net dist

src/logo_data.h: src/logo.png tools/png2c.py
	python3 tools/png2c.py src/logo.png src/logo_data.h 280 280

src/wallpaper_data.h: src/Library/wallpapers/moonosalpha.png tools/png2c.py
	python3 tools/png2c.py src/Library/wallpapers/moonosalpha.png src/wallpaper_data.h 960 540 WALLPAPER

src/cursor_data.h: src/Library/cursor/arrow.cur tools/cur2c.py
	python3 tools/cur2c.py src/Library/cursor/arrow.cur src/cursor_data.h 32 32

src/terminal_icon_data.h: tools/png2c.py
	@if [ -f "src/Library/apps/terminal/Terminal.png" ]; then \
		python3 tools/png2c.py src/Library/apps/terminal/Terminal.png $@ 64 64 TERMINAL_ICON; \
	else \
		python3 tools/png2c.py src/logo.png $@ 64 64 TERMINAL_ICON; \
	fi

src/arcpad_icon_data.h: tools/png2c.py
	@if [ -f "src/Library/apps/arcpad/arcpad.png" ]; then \
		python3 tools/png2c.py src/Library/apps/arcpad/arcpad.png $@ 64 64 ARCPAD_ICON; \
	else \
		python3 tools/png2c.py src/logo.png $@ 64 64 ARCPAD_ICON; \
	fi

src/settings_icon_data.h: tools/png2c.py
	@for f in settings.png icon.png; do \
		if [ -f "src/Library/apps/settings/$$f" ]; then \
			python3 tools/png2c.py "src/Library/apps/settings/$$f" $@ 64 64 SETTINGS_ICON; \
			exit 0; \
		fi; \
	done; \
	python3 tools/png2c.py src/logo.png $@ 64 64 SETTINGS_ICON

src/gui_font.h: src/Library/fonts/SF-Pro-Display-Regular.otf tools/otf2bitmapfont.py
	python3 tools/otf2bitmapfont.py $< $@ 22

GENERATED_HEADERS = src/logo_data.h src/wallpaper_data.h src/cursor_data.h src/terminal_icon_data.h src/arcpad_icon_data.h src/settings_icon_data.h src/gui_font.h

build/apps/%.o: src/apps/%.c $(GENERATED_HEADERS) | build
	$(CC) $(CFLAGS) -c $< -o $@
	
# No internet for now..
# build/net/%.o: src/net/%.c $(GENERATED_HEADERS) | build
#	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c $(GENERATED_HEADERS) | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.asm | build
	$(AS) $(ASFLAGS) $< -o $@

build/moonos.elf: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# ── ISO assembly ────────────────────────────────────────────────────────────
src/iso_root/boot/moonos.elf: build/moonos.elf
	cp $< $@

src/iso_root/boot/background.png: src/background.png
	cp $< $@
	cp $< src/iso_root/background.png

src/iso_root/boot/limine/limine-uefi-cd.bin: $(LIMINE_DIR)/limine-uefi-cd.bin
	cp $< src/iso_root/boot/limine/

src/iso_root/boot/limine/limine-bios-cd.bin: $(LIMINE_DIR)/limine-bios-cd.bin
	cp $< src/iso_root/boot/limine/

src/iso_root/boot/limine/limine-bios.sys: $(LIMINE_DIR)/limine-bios.sys
	cp $< src/iso_root/boot/limine/

src/iso_root/EFI/BOOT/BOOTX64.EFI: $(LIMINE_DIR)/BOOTX64.EFI
	cp $< src/iso_root/EFI/BOOT/

LIMINE_BINS = src/iso_root/boot/limine/limine-uefi-cd.bin \
              src/iso_root/boot/limine/limine-bios-cd.bin  \
              src/iso_root/boot/limine/limine-bios.sys     \
              src/iso_root/EFI/BOOT/BOOTX64.EFI

$(LIMINE_DIR)/limine:
	$(MAKE) -C $(LIMINE_DIR) limine

$(ISO_MAIN): src/iso_root/boot/moonos.elf src/iso_root/boot/background.png $(LIMINE_BINS) $(LIMINE_DIR)/limine
	mkdir -p dist
	xorriso -as mkisofs \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		src/iso_root -o $(ISO_MAIN)
	$(LIMINE_DIR)/limine bios-install $(ISO_MAIN)
	cp $(ISO_MAIN) $(ISO_VER)
	@echo "==> Built: $(ISO_MAIN)  ($(ISO_VER))"

clean:
	rm -rf build dist/
	rm -f src/iso_root/boot/moonos.elf src/iso_root/boot/background.png src/iso_root/background.png src/logo_data.h src/wallpaper_data.h src/cursor_data.h src/terminal_icon_data.h src/arcpad_icon_data.h src/settings_icon_data.h src/gui_font.h

# ── QEMU quick test ─────────────────────────────────────────────────────────
OVMF     := /usr/share/ovmf/OVMF.fd

run: $(ISO_MAIN)
	@if [ -f "$(OVMF)" ]; then \
		SDL_VIDEO_WINDOW_POS=$(MONITOR2),0 \
		qemu-system-x86_64 -cdrom $(ISO_MAIN) -m 512M -vga std \
			-display sdl -full-screen \
			-bios $(OVMF) -no-reboot; \
	else \
		SDL_VIDEO_WINDOW_POS=$(MONITOR2),0 \
		qemu-system-x86_64 -cdrom $(ISO_MAIN) -m 512M -vga std \
			-display sdl -full-screen \
			-no-reboot; \
	fi
