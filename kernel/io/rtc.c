// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "rtc.h"
#include "io.h"

static uint8_t rtc_reg(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd2bin(uint8_t v) { return (v >> 4) * 10 + (v & 0xF); }

void rtc_get(int dt[6]) {
    uint8_t status = rtc_reg(0x0B);
    dt[0] = bcd2bin(rtc_reg(0x09));
    dt[1] = bcd2bin(rtc_reg(0x08));
    dt[2] = bcd2bin(rtc_reg(0x07));
    dt[3] = bcd2bin(rtc_reg(0x04));
    dt[4] = bcd2bin(rtc_reg(0x02));
    dt[5] = bcd2bin(rtc_reg(0x00));
    if (!(status & 0x04)) {
        dt[0] = (dt[0] & 0x0F) + ((dt[0] / 16) * 10);
    }
}
