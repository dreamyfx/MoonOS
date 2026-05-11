// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void     pmm_init(void *memmap, uint64_t count, uint64_t hhdm);
uint64_t pmm_alloc(void);
void     pmm_free(uint64_t addr);
uint64_t pmm_total(void);
uint64_t pmm_used(void);
