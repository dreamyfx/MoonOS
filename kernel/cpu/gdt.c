// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "gdt.h"
#include "../lib/kstring.h"

#define GDT_ENTRIES 7

static gdt_entry_t g_gdt[GDT_ENTRIES];
static tss_t       g_tss;

typedef struct __attribute__((packed)) { uint16_t limit; uint64_t base; } gdtr_t;

static void gdt_set(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    g_gdt[idx].base_low  = base & 0xFFFF;
    g_gdt[idx].base_mid  = (base >> 16) & 0xFF;
    g_gdt[idx].base_high = (base >> 24) & 0xFF;
    g_gdt[idx].limit_low = limit & 0xFFFF;
    g_gdt[idx].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    g_gdt[idx].access    = access;
}

extern void gdt_flush(uint64_t gdtr_ptr);
extern void tss_flush(void);

void gdt_init(void) {
    k_memset(&g_tss, 0, sizeof(g_tss));
    g_tss.iopb = sizeof(g_tss);

    gdt_set(0, 0, 0, 0x00, 0x00);
    gdt_set(1, 0, 0xFFFFF, 0x9A, 0xA0);
    gdt_set(2, 0, 0xFFFFF, 0x92, 0xC0);
    gdt_set(3, 0, 0xFFFFF, 0xF2, 0xC0);
    gdt_set(4, 0, 0xFFFFF, 0xFA, 0xA0);

    uint64_t tss_base = (uint64_t)&g_tss;
    uint32_t tss_limit = sizeof(g_tss) - 1;
    g_gdt[5].limit_low  = tss_limit & 0xFFFF;
    g_gdt[5].base_low   = tss_base & 0xFFFF;
    g_gdt[5].base_mid   = (tss_base >> 16) & 0xFF;
    g_gdt[5].access     = 0x89;
    g_gdt[5].gran       = ((tss_limit >> 16) & 0x0F);
    g_gdt[5].base_high  = (tss_base >> 24) & 0xFF;
    *(uint32_t *)&g_gdt[6] = (tss_base >> 32) & 0xFFFFFFFF;
    *(uint32_t *)((uint8_t *)&g_gdt[6] + 4) = 0;

    gdtr_t gdtr = { .limit = sizeof(g_gdt) - 1, .base = (uint64_t)g_gdt };
    gdt_flush((uint64_t)&gdtr);
    tss_flush();
}

void gdt_set_tss_rsp0(uint64_t rsp0) {
    g_tss.rsp0 = rsp0;
}
