// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>

#define GDT_NULL       0x00
#define GDT_KCODE      0x08
#define GDT_KDATA      0x10
#define GDT_UDATA      0x18
#define GDT_UCODE      0x20
#define GDT_TSS_LOW    0x28

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} gdt_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t res1;
    uint64_t ist[7];
    uint64_t res2;
    uint16_t res3;
    uint16_t iopb;
} tss_t;

void gdt_init(void);
void gdt_set_tss_rsp0(uint64_t rsp0);
