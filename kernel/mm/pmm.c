// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "pmm.h"
#include "../lib/kstring.h"
#include "../cpu/spinlock.h"
#include "../boot/limine.h"

#define MAX_FRAMES 131072

static uint64_t g_hhdm;
static uint8_t  g_bitmap[MAX_FRAMES / 8];
static uint64_t g_total_frames;
static uint64_t g_used_frames;
static uint64_t g_base_addr;
static spinlock_t g_lock = SPINLOCK_INIT;

static void bitmap_set(uint64_t idx) { g_bitmap[idx / 8] |= (1 << (idx % 8)); }
static void bitmap_clr(uint64_t idx) { g_bitmap[idx / 8] &= ~(1 << (idx % 8)); }
static int  bitmap_get(uint64_t idx) { return (g_bitmap[idx / 8] >> (idx % 8)) & 1; }

void pmm_init(void *memmap_resp, uint64_t count, uint64_t hhdm) {
    g_hhdm = hhdm;
    k_memset(g_bitmap, 0xFF, sizeof(g_bitmap));
    g_total_frames = 0;
    g_used_frames  = 0;
    g_base_addr    = 0;

    struct limine_memmap_response *resp = memmap_resp;
    uint64_t first_base = 0;
    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *e = resp->entries[i];
        if (e->type == LIMINE_MEMMAP_USABLE && first_base == 0) {
            first_base = e->base;
        }
    }
    g_base_addr = first_base;

    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *e = resp->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t frames = e->length / PAGE_SIZE;
        uint64_t start  = (e->base - g_base_addr) / PAGE_SIZE;
        for (uint64_t f = 0; f < frames && (start + f) < MAX_FRAMES; f++) {
            bitmap_clr(start + f);
            g_total_frames++;
        }
    }
}

uint64_t pmm_alloc(void) {
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    for (uint64_t i = 0; i < g_total_frames; i++) {
        if (!bitmap_get(i)) {
            bitmap_set(i);
            g_used_frames++;
            uint64_t addr = g_base_addr + i * PAGE_SIZE;
            k_memset((void *)(addr + g_hhdm), 0, PAGE_SIZE);
            spinlock_release_irqrestore(&g_lock, fl);
            return addr;
        }
    }
    spinlock_release_irqrestore(&g_lock, fl);
    return 0;
}

void pmm_free(uint64_t addr) {
    if (addr < g_base_addr) return;
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    uint64_t idx = (addr - g_base_addr) / PAGE_SIZE;
    if (idx < g_total_frames && bitmap_get(idx)) {
        bitmap_clr(idx);
        g_used_frames--;
    }
    spinlock_release_irqrestore(&g_lock, fl);
}

uint64_t pmm_total(void) { return g_total_frames * PAGE_SIZE; }
uint64_t pmm_used(void)  { return g_used_frames  * PAGE_SIZE; }
