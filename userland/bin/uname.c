// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "../libc/stdlib.h"
#include "../libc/syscall.h"

static void parse_version(const char *buf, char *kern, char *ver, char *arch) {
    int line = 0, i = 0;
    while (buf[i]) {
        int start = i;
        while (buf[i] && buf[i] != '\n') i++;
        int end = i;
        if (buf[i]) i++;
        char tmp[256];
        int len = end - start < 255 ? end - start : 255;
        strncpy(tmp, buf + start, len); tmp[len] = 0;

        if (line == 0) {
            strncpy(kern, tmp, 63);
        } else if (line == 1) {
            char *sp = strchr(tmp, ' ');
            if (sp) {
                *sp = 0;
                strncpy(ver, sp + 1, 63);
            }
        } else if (line == 2) {
            strncpy(arch, tmp, 63);
        }
        line++;
    }
}

int main(int argc, char **argv) {
    int want_s=0, want_n=0, want_r=0, want_v=0, want_m=0, want_p=0, want_o=0;
    int any = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (a[0] != '-') { printf("Usage: uname [-amnoprsv]\n"); return 1; }
        for (int j = 1; a[j]; j++) {
            switch (a[j]) {
            case 'a': want_s=want_n=want_r=want_v=want_m=want_p=want_o=1; any=1; break;
            case 's': want_s=1; any=1; break;
            case 'n': want_n=1; any=1; break;
            case 'r': want_r=1; any=1; break;
            case 'v': want_v=1; any=1; break;
            case 'm': want_m=1; any=1; break;
            case 'p': want_p=1; any=1; break;
            case 'o': want_o=1; any=1; break;
            default: printf("uname: invalid option -%c\n", a[j]); return 1;
            }
        }
    }
    if (!any) want_s = 1;

    char vbuf[1024] = {0};
    char kern[64]   = "MoonKernel";
    char ver[64]    = "1.0.0-release";
    char arch[64]   = "x86_64";
    const char *node = "moonos";

    int fd = sys_open("/proc/version", 0);
    if (fd >= 0) {
        int n = sys_read(fd, vbuf, sizeof(vbuf)-1);
        sys_close(fd);
        if (n > 0) { vbuf[n] = 0; parse_version(vbuf, kern, ver, arch); }
    }

    int first = 1;
#define FIELD(cond, val) if (cond) { if (!first) sys_write(1, " ", 1); sys_write(1, val, strlen(val)); first = 0; }
    FIELD(want_s, kern);
    FIELD(want_n, node);
    FIELD(want_r, ver);
    FIELD(want_v, ver);
    FIELD(want_m, arch);
    FIELD(want_p, arch);
    FIELD(want_o, "MoonOS");
    sys_write(1, "\n", 1);
    return 0;
}
