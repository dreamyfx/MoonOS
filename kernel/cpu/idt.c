// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "idt.h"
#include "../lib/kstring.h"
#include "../lib/console.h"
#include "../lib/panic.h"
#include "../io/io.h"

typedef struct __attribute__((packed)) {
    uint16_t off_lo;
    uint16_t sel;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t off_mid;
    uint32_t off_hi;
    uint32_t zero;
} idt_entry_t;

typedef struct __attribute__((packed)) { uint16_t limit; uint64_t base; } idtr_t;

#define IDT_SIZE 256
static idt_entry_t g_idt[IDT_SIZE];
static idtr_t      g_idtr;

extern void *isr_stubs[48];

void idt_set_handler(uint8_t vec, void *handler, uint8_t ist) {
    uint64_t addr = (uint64_t)handler;
    g_idt[vec].off_lo  = addr & 0xFFFF;
    g_idt[vec].sel     = 0x08;
    g_idt[vec].ist     = ist & 0x7;
    g_idt[vec].flags   = 0x8E;
    g_idt[vec].off_mid = (addr >> 16) & 0xFFFF;
    g_idt[vec].off_hi  = (addr >> 32) & 0xFFFFFFFF;
    g_idt[vec].zero    = 0;
}

void idt_load(void) {
    g_idtr.limit = sizeof(g_idt) - 1;
    g_idtr.base  = (uint64_t)g_idt;
    asm volatile("lidt %0" : : "m"(g_idtr));
}

void isr_handler(cpu_regs_t *r);

void isr_handler(cpu_regs_t *r) {
    if (r->vec < 32) {
        static const char *exc[] = {
            "Divide By Zero","Debug","NMI","Breakpoint","Overflow","Bound Range",
            "Invalid Opcode","Device NA","Double Fault","Coprocessor Seg","Invalid TSS",
            "Segment NP","Stack Fault","GPF","Page Fault","Reserved","x87 FP","Alignment",
            "Machine Check","SIMD FP","Virt","Control Prot"
        };
        const char *name = (r->vec < 22) ? exc[r->vec] : "Unknown Exception";
        console_printf("\n[EXCEPTION %d] %s err=%x rip=%x\n",
            (int)r->vec, name, r->err, r->rip);
        k_panic("unhandled exception");
    }
    outb(0x20, 0x20);
    if (r->vec >= 0x28) outb(0xA0, 0x20);

    extern void irq_dispatch(uint8_t irq);
    irq_dispatch((uint8_t)(r->vec - 0x20));
}

void idt_init(void) {
    k_memset(g_idt, 0, sizeof(g_idt));
    for (int i = 0; i < 48; i++) {
        idt_set_handler((uint8_t)i, isr_stubs[i], 0);
    }
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFD); outb(0xA1, 0xFF);
}
