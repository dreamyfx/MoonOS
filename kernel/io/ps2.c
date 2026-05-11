// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "ps2.h"
#include "io.h"
#include "../tty/tty.h"
#include "../lib/console.h"
#include <stdint.h>

static int g_shift = 0;
static int g_caps  = 0;

static const char sc_map_lo[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const char sc_map_hi[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char ps2_scancode_to_ascii(uint8_t sc, int shift) {
    if (sc >= 128) return 0;
    return shift ? sc_map_hi[sc] : sc_map_lo[sc];
}

void ps2_irq(void) {
    uint8_t sc = inb(0x60);
    if (sc & 0x80) {
        uint8_t rel = sc & 0x7F;
        if (rel == 0x2A || rel == 0x36) g_shift = 0;
        return;
    }
    if (sc == 0x2A || sc == 0x36) { g_shift = 1; return; }
    if (sc == 0x3A) { g_caps ^= 1; return; }

    int effective_shift = g_shift ^ (g_caps && sc >= 0x10 && sc <= 0x32);
    char c = ps2_scancode_to_ascii(sc, effective_shift);
    if (c) {
        if (c == '\b') {
            console_putc('\b');
        }
        tty_input_char(c);
    }
    if (sc == 0x48) tty_input_char('\x11');
    if (sc == 0x50) tty_input_char('\x12');
    if (sc == 0x4D) tty_input_char('\x13');
    if (sc == 0x4B) tty_input_char('\x14');
}

void ps2_init(void) {
    while (inb(0x64) & 1) inb(0x60);
}
