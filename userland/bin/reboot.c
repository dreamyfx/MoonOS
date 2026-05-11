// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "../libc/stdlib.h"
#include "../libc/syscall.h"

int main(void) {
    sys_write(1, "Rebooting MoonOS...\n", 20);
    sys_reboot();
    return 0;
}
