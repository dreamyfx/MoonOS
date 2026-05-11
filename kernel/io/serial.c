// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "serial.h"
#include "io.h"

#define COM1 0x3F8

void serial_init(void) {
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x80);
    outb(COM1+0, 0x03);
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x03);
    outb(COM1+2, 0xC7);
    outb(COM1+4, 0x0B);
}

void serial_write(const char *s) {
    while (*s) {
        while (!(inb(COM1+5) & 0x20));
        outb(COM1, *s++);
    }
}
