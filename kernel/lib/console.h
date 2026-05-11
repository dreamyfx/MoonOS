// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stddef.h>

void console_init(uint32_t *fb, uint32_t w, uint32_t h, uint32_t pitch);
void console_putc(char c);
void console_puts(const char *s);
void console_printf(const char *fmt, ...);
void console_set_color(uint32_t fg);
void console_clear(void);
void console_put_hex(uint64_t n);
void console_put_dec(int64_t n);
