// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "panic.h"
#include "console.h"

void k_panic(const char *msg) {
    console_set_color(0xFFFF2222);
    console_puts("\n[PANIC] ");
    console_puts(msg);
    console_puts("\n");
    asm volatile("cli");
    for (;;) asm volatile("hlt");
}
