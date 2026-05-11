// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "lapic.h"
#include "../lib/kstring.h"

#define IA32_APIC_BASE_MSR  0x1B
#define LAPIC_ID            0x020
#define LAPIC_EOI           0x0B0
#define LAPIC_SIVR          0x0F0
#define LAPIC_ICR_LO        0x300
#define LAPIC_ICR_HI        0x310
#define LAPIC_TIMER_LVT     0x320
#define LAPIC_TIMER_INIT    0x380
#define LAPIC_TIMER_CUR     0x390
#define LAPIC_TIMER_DIV     0x3E0

static volatile uint32_t *g_lapic;
static uint32_t g_ticks;

static uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static uint32_t lapic_read(uint32_t reg) { return g_lapic[reg / 4]; }
static void     lapic_write(uint32_t reg, uint32_t val) { g_lapic[reg / 4] = val; }

void lapic_init(void) {
    uint64_t base = rdmsr(IA32_APIC_BASE_MSR) & ~0xFFFULL;
    extern uint64_t g_hhdm_offset;
    g_lapic = (volatile uint32_t *)(base + g_hhdm_offset);
    lapic_write(LAPIC_SIVR, lapic_read(LAPIC_SIVR) | 0x1FF);
    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_TIMER_LVT, 0x20000 | 0x20);
    lapic_write(LAPIC_TIMER_INIT, 10000000);
}

void lapic_eoi(void) { lapic_write(LAPIC_EOI, 0); }

uint32_t lapic_id(void) { return lapic_read(LAPIC_ID) >> 24; }

void lapic_send_ipi(uint32_t apic_id, uint8_t vec) {
    lapic_write(LAPIC_ICR_HI, apic_id << 24);
    lapic_write(LAPIC_ICR_LO, vec);
}

uint32_t lapic_get_ticks(void) { return g_ticks; }

void lapic_timer_tick(void) { g_ticks++; lapic_eoi(); }
