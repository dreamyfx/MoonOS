// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "vmm.h"
#include "pmm.h"
#include "../lib/kstring.h"

static uint64_t g_hhdm;
static pagemap_t g_kernel_pm;

static uint64_t *pte_get_or_alloc(uint64_t *table, uint16_t idx, uint64_t flags) {
    if (!(table[idx] & VMM_PRESENT)) {
        uint64_t phys = pmm_alloc();
        table[idx] = phys | flags;
    }
    return (uint64_t *)((table[idx] & ~0xFFFULL) + g_hhdm);
}

void vmm_init(uint64_t hhdm) {
    g_hhdm = hhdm;
    g_kernel_pm = pmm_alloc();
    uint64_t *pml4 = (uint64_t *)(g_kernel_pm + hhdm);
    k_memset(pml4, 0, PAGE_SIZE);

    for (uint64_t i = 0; i < 0x100000000ULL; i += PAGE_SIZE) {
        vmm_map(g_kernel_pm, i + hhdm, i, VMM_PRESENT | VMM_WRITE);
        vmm_map(g_kernel_pm, i, i, VMM_PRESENT | VMM_WRITE);
    }
    vmm_switch(g_kernel_pm);
}

void vmm_map(pagemap_t pm, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t *pml4 = (uint64_t *)(pm + g_hhdm);
    uint16_t i4 = (virt >> 39) & 0x1FF;
    uint16_t i3 = (virt >> 30) & 0x1FF;
    uint16_t i2 = (virt >> 21) & 0x1FF;
    uint16_t i1 = (virt >> 12) & 0x1FF;
    uint64_t *pdp = pte_get_or_alloc(pml4, i4, VMM_PRESENT | VMM_WRITE | VMM_USER);
    uint64_t *pd  = pte_get_or_alloc(pdp,  i3, VMM_PRESENT | VMM_WRITE | VMM_USER);
    uint64_t *pt  = pte_get_or_alloc(pd,   i2, VMM_PRESENT | VMM_WRITE | VMM_USER);
    pt[i1] = (phys & ~0xFFFULL) | flags;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_unmap(pagemap_t pm, uint64_t virt) {
    uint64_t *pml4 = (uint64_t *)(pm + g_hhdm);
    uint16_t i4 = (virt >> 39) & 0x1FF;
    uint16_t i3 = (virt >> 30) & 0x1FF;
    uint16_t i2 = (virt >> 21) & 0x1FF;
    uint16_t i1 = (virt >> 12) & 0x1FF;
    if (!(pml4[i4] & VMM_PRESENT)) return;
    uint64_t *pdp = (uint64_t *)((pml4[i4] & ~0xFFFULL) + g_hhdm);
    if (!(pdp[i3] & VMM_PRESENT)) return;
    uint64_t *pd  = (uint64_t *)((pdp[i3] & ~0xFFFULL) + g_hhdm);
    if (!(pd[i2] & VMM_PRESENT)) return;
    uint64_t *pt  = (uint64_t *)((pd[i2] & ~0xFFFULL) + g_hhdm);
    pt[i1] = 0;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_switch(pagemap_t pm) {
    asm volatile("mov %0, %%cr3" : : "r"(pm) : "memory");
}

uint64_t vmm_translate(pagemap_t pm, uint64_t virt) {
    uint64_t *pml4 = (uint64_t *)(pm + g_hhdm);
    uint16_t i4 = (virt >> 39) & 0x1FF;
    uint16_t i3 = (virt >> 30) & 0x1FF;
    uint16_t i2 = (virt >> 21) & 0x1FF;
    uint16_t i1 = (virt >> 12) & 0x1FF;
    if (!(pml4[i4] & VMM_PRESENT)) return 0;
    uint64_t *pdp = (uint64_t *)((pml4[i4] & ~0xFFFULL) + g_hhdm);
    if (!(pdp[i3] & VMM_PRESENT)) return 0;
    uint64_t *pd  = (uint64_t *)((pdp[i3] & ~0xFFFULL) + g_hhdm);
    if (!(pd[i2] & VMM_PRESENT)) return 0;
    uint64_t *pt  = (uint64_t *)((pd[i2] & ~0xFFFULL) + g_hhdm);
    if (!(pt[i1] & VMM_PRESENT)) return 0;
    return (pt[i1] & ~0xFFFULL) | (virt & 0xFFF);
}

pagemap_t vmm_new(void) {
    pagemap_t pm = pmm_alloc();
    uint64_t *pml4 = (uint64_t *)(pm + g_hhdm);
    uint64_t *kpml4 = (uint64_t *)(g_kernel_pm + g_hhdm);
    k_memset(pml4, 0, PAGE_SIZE);
    for (int i = 256; i < 512; i++) pml4[i] = kpml4[i];
    return pm;
}

void vmm_destroy(pagemap_t pm) {
    pmm_free(pm);
}

pagemap_t vmm_fork(pagemap_t src) {
    (void)src;
    return vmm_new();
}

pagemap_t vmm_kernel_pm(void) { return g_kernel_pm; }
