// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "limine.h"
#include "../lib/console.h"
#include "../lib/kstring.h"
#include "../lib/panic.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/lapic.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../proc/process.h"
#include "../syscall/syscall.h"
#include "../fs/vfs.h"
#include "../fs/tarfs.h"
#include "../fs/procfs.h"
#include "../tty/tty.h"
#include "../io/ps2.h"
#include "../io/serial.h"

uint64_t g_hhdm_offset;

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 1
};

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request mm_req = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST, .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_module_request mod_req = {
    .id = LIMINE_MODULE_REQUEST, .revision = 0
};

__attribute__((used, section(".requests_start")))
static volatile struct limine_framebuffer_request *const rs[] = { &fb_req, NULL };

__attribute__((used, section(".requests_end")))
static volatile struct limine_framebuffer_request *const re[] = { NULL };

static void hcf(void) { asm("cli"); for (;;) asm("hlt"); }

void kmain(void) {
    serial_init();
    serial_write("[MoonOS] Booting...\n");

    if (hhdm_req.response)
        g_hhdm_offset = hhdm_req.response->offset;

    if (!fb_req.response || !fb_req.response->framebuffer_count) {
        serial_write("[FAIL] No framebuffer\n");
        hcf();
    }

    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
    console_init((uint32_t *)fb->address, (uint32_t)fb->width,
                 (uint32_t)fb->height,   (uint32_t)fb->pitch);

    console_set_color(0xFF88AAFF);
    console_puts("  __  __                   ___  ____  \n");
    console_puts(" |  \\/  | ___   ___  _ __  / _ \\/ ___| \n");
    console_puts(" | |\\/| |/ _ \\ / _ \\| '_ \\| | | \\___ \\ \n");
    console_puts(" | |  | | (_) | (_) | | | | |_| |___) |\n");
    console_puts(" |_|  |_|\\___/ \\___/|_| |_|\\___/|____/ \n");
    console_set_color(0xFFCCCCCC);
    console_puts(" MoonOS 1.0.0 x86_64  |  (c) 2025-2026 Andrew (dreamyfx)\n\n");

    if (!mm_req.response) k_panic("No memory map");
    pmm_init(mm_req.response, mm_req.response->entry_count, g_hhdm_offset);
    console_printf("[OK] PMM: %d MB total\n", (int)(pmm_total() / 1048576));

    vmm_init(g_hhdm_offset);
    console_puts("[OK] VMM ready\n");

    heap_init();
    console_puts("[OK] Heap ready\n");

    gdt_init();
    console_puts("[OK] GDT ready\n");

    idt_init();
    idt_load();
    console_puts("[OK] IDT ready\n");

    lapic_init();
    console_puts("[OK] LAPIC ready\n");

    asm volatile("sti");
    ps2_init();
    console_puts("[OK] PS2 ready\n");

    vfs_init();
    tty_init();
    proc_init();
    syscall_init();
    console_puts("[OK] Kernel subsystems ready\n");

    vfs_mount("/proc", procfs_ops());

    if (mod_req.response && mod_req.response->module_count > 0) {
        struct limine_file *mod = mod_req.response->modules[0];
        tarfs_init(mod->address, mod->size);
        vfs_mount("/", tarfs_ops());
        console_puts("[OK] Initrd mounted at /\n");
    } else {
        console_puts("[WARN] No initrd module found\n");
    }

    console_set_color(0xFF88FFAA);
    console_puts("\nStarting msh...\n\n");
    console_set_color(0xFFCCCCCC);

    int tty0 = 0;
    process_t *shell = proc_spawn("/bin/msh.elf", NULL, tty0);
    if (!shell) {
        console_puts("[FAIL] Cannot start msh - check initrd\n");
        hcf();
    }

    extern void proc_run_first(process_t *p);
    proc_run_first(shell);

    hcf();
}
