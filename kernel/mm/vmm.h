// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define VMM_PRESENT  (1ULL << 0)
#define VMM_WRITE    (1ULL << 1)
#define VMM_USER     (1ULL << 2)
#define VMM_NX       (1ULL << 63)

typedef uint64_t pagemap_t;

void      vmm_init(uint64_t hhdm);
pagemap_t vmm_new(void);
void      vmm_map(pagemap_t pm, uint64_t virt, uint64_t phys, uint64_t flags);
void      vmm_unmap(pagemap_t pm, uint64_t virt);
void      vmm_switch(pagemap_t pm);
pagemap_t vmm_fork(pagemap_t src);
void      vmm_destroy(pagemap_t pm);
uint64_t  vmm_translate(pagemap_t pm, uint64_t virt);
pagemap_t vmm_kernel_pm(void);
