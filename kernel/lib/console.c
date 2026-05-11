// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "console.h"
#include "kstring.h"
#include "../cpu/spinlock.h"
#include <stdarg.h>

#define FONT_W 8
#define FONT_H 16
#define TAB_W  4

static const uint8_t font8x16[128][16] = {
#include "font8x16_data.h"
};

static uint32_t *g_fb;
static uint32_t  g_width, g_height, g_pitch_px;
static uint32_t  g_cols, g_rows;
static uint32_t  g_cx, g_cy;
static uint32_t  g_fg = 0xFFCCCCCC;
static spinlock_t g_lock = SPINLOCK_INIT;

void console_init(uint32_t *fb, uint32_t w, uint32_t h, uint32_t pitch) {
    g_fb = fb;
    g_width = w; g_height = h;
    g_pitch_px = pitch / 4;
    g_cols = w / FONT_W;
    g_rows = h / FONT_H;
    g_cx = g_cy = 0;
    console_clear();
}

void console_set_color(uint32_t fg) { g_fg = fg; }

void console_clear(void) {
    if (!g_fb) return;
    for (uint32_t i = 0; i < g_height; i++)
        for (uint32_t j = 0; j < g_width; j++)
            g_fb[i * g_pitch_px + j] = 0xFF000000;
    g_cx = g_cy = 0;
}

static void scroll_up(void) {
    uint32_t row_bytes = FONT_H * g_pitch_px;
    k_memcpy(g_fb, g_fb + row_bytes, (g_rows - 1) * FONT_H * g_pitch_px * 4);
    uint32_t *last = g_fb + (g_rows - 1) * FONT_H * g_pitch_px;
    for (uint32_t i = 0; i < FONT_H * g_pitch_px; i++) last[i] = 0xFF000000;
    if (g_cy > 0) g_cy--;
}

static void draw_char(char c, uint32_t col, uint32_t row) {
    if ((unsigned char)c >= 128) c = '?';
    const uint8_t *glyph = font8x16[(unsigned char)c];
    uint32_t base_x = col * FONT_W;
    uint32_t base_y = row * FONT_H;
    for (int y = 0; y < FONT_H; y++) {
        uint8_t bits = glyph[y];
        for (int x = 0; x < FONT_W; x++) {
            uint32_t px = g_fb[(base_y + y) * g_pitch_px + (base_x + x)];
            (void)px;
            g_fb[(base_y + y) * g_pitch_px + (base_x + x)] =
                (bits & (0x80 >> x)) ? g_fg : 0xFF000000;
        }
    }
}

void console_putc(char c) {
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    if (!g_fb) { spinlock_release_irqrestore(&g_lock, fl); return; }
    if (c == '\n') {
        g_cx = 0; g_cy++;
    } else if (c == '\r') {
        g_cx = 0;
    } else if (c == '\t') {
        g_cx = (g_cx + TAB_W) & ~(TAB_W - 1);
    } else if (c == '\b') {
        if (g_cx > 0) {
            g_cx--;
            draw_char(' ', g_cx, g_cy);
        }
    } else {
        draw_char(c, g_cx, g_cy);
        g_cx++;
        if (g_cx >= g_cols) { g_cx = 0; g_cy++; }
    }
    while (g_cy >= g_rows) scroll_up();
    spinlock_release_irqrestore(&g_lock, fl);
}

void console_puts(const char *s) {
    while (*s) console_putc(*s++);
}

void console_put_hex(uint64_t n) {
    console_puts("0x");
    char buf[17]; int i = 15;
    buf[16] = 0;
    if (n == 0) { console_puts("0"); return; }
    while (n) {
        int d = n & 0xF;
        buf[i--] = d < 10 ? '0' + d : 'A' + d - 10;
        n >>= 4;
    }
    console_puts(buf + i + 1);
}

void console_put_dec(int64_t n) {
    if (n < 0) { console_putc('-'); n = -n; }
    char buf[21]; int i = 19;
    buf[20] = 0;
    if (n == 0) { console_putc('0'); return; }
    while (n) { buf[i--] = '0' + (n % 10); n /= 10; }
    console_puts(buf + i + 1);
}

void console_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { console_putc(*p); continue; }
        p++;
        switch (*p) {
        case 's': { const char *s = va_arg(ap, const char *); console_puts(s ? s : "(null)"); break; }
        case 'd': { int n = va_arg(ap, int); console_put_dec(n); break; }
        case 'u': { unsigned n = va_arg(ap, unsigned); console_put_dec(n); break; }
        case 'x': { unsigned long long n = va_arg(ap, unsigned long long); console_put_hex(n); break; }
        case 'c': { char c = (char)va_arg(ap, int); console_putc(c); break; }
        case '%': console_putc('%'); break;
        default: console_putc('%'); console_putc(*p); break;
        }
    }
    va_end(ap);
}
