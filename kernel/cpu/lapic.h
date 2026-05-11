// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>

void    lapic_init(void);
void    lapic_eoi(void);
uint32_t lapic_id(void);
void    lapic_send_ipi(uint32_t apic_id, uint8_t vec);
uint32_t lapic_get_ticks(void);
