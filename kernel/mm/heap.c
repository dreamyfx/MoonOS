// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "../lib/kstring.h"
#include "../cpu/spinlock.h"

#define HEAP_BASE  0xFFFF900000000000ULL
#define HEAP_PAGES 4096
#define BLOCK_MAGIC 0xDEADC0DEUL

typedef struct block {
    uint32_t magic;
    uint32_t size;
    uint8_t  free;
    struct block *next;
} block_t;

static block_t *g_head;
static spinlock_t g_lock = SPINLOCK_INIT;

void heap_init(void) {
    for (uint64_t i = 0; i < HEAP_PAGES; i++) {
        uint64_t phys = pmm_alloc();
        vmm_map(vmm_kernel_pm(), HEAP_BASE + i * PAGE_SIZE, phys,
                VMM_PRESENT | VMM_WRITE);
    }
    g_head = (block_t *)HEAP_BASE;
    g_head->magic = BLOCK_MAGIC;
    g_head->size  = HEAP_PAGES * PAGE_SIZE - sizeof(block_t);
    g_head->free  = 1;
    g_head->next  = 0;
}

void *kmalloc(size_t size) {
    if (!size) return 0;
    size = (size + 7) & ~7ULL;
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    block_t *b = g_head;
    while (b) {
        if (b->free && b->size >= size) {
            if (b->size > size + sizeof(block_t) + 8) {
                block_t *nb = (block_t *)((uint8_t *)b + sizeof(block_t) + size);
                nb->magic = BLOCK_MAGIC;
                nb->size  = b->size - size - sizeof(block_t);
                nb->free  = 1;
                nb->next  = b->next;
                b->next   = nb;
                b->size   = size;
            }
            b->free = 0;
            spinlock_release_irqrestore(&g_lock, fl);
            return (uint8_t *)b + sizeof(block_t);
        }
        b = b->next;
    }
    spinlock_release_irqrestore(&g_lock, fl);
    return 0;
}

void *kcalloc(size_t n, size_t size) {
    void *p = kmalloc(n * size);
    if (p) k_memset(p, 0, n * size);
    return p;
}

void kfree(void *ptr) {
    if (!ptr) return;
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    block_t *b = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    if (b->magic == BLOCK_MAGIC) b->free = 1;
    spinlock_release_irqrestore(&g_lock, fl);
}

void *krealloc(void *ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    block_t *b = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    if (b->size >= size) return ptr;
    void *np = kmalloc(size);
    if (np) { k_memcpy(np, ptr, b->size); kfree(ptr); }
    return np;
}
