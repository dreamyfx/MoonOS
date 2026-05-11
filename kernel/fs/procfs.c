// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "procfs.h"
#include "../lib/kstring.h"
#include "../mm/pmm.h"

#define MAX_PROC_FDS 16

typedef struct { char buf[1024]; uint32_t off; uint32_t len; } proc_fd_t;
static proc_fd_t g_pfds[MAX_PROC_FDS];

static void build_version(char *buf, int max) {
    int n = 0;
    const char *os   = "MoonOS 1.0.0";
    const char *kern = "MoonKernel 1.0.0-release";
    const char *arch = "x86_64";
    const char *date = __DATE__;
    n += k_strlen(os);   if (n < max) { k_strcpy(buf, os); }
    buf[n++] = '\n';
    k_strncpy(buf + n, kern, max - n); n += k_strlen(kern);
    buf[n++] = '\n';
    k_strncpy(buf + n, arch, max - n); n += k_strlen(arch);
    buf[n++] = '\n';
    k_strncpy(buf + n, date, max - n); n += k_strlen(date);
    buf[n++] = '\n';
    buf[n] = 0;
}

static void build_meminfo(char *buf, int max) {
    uint64_t total = pmm_total() / 1024;
    uint64_t used  = pmm_used()  / 1024;
    uint64_t free  = total - used;
    char tmp[32];
    k_strcpy(buf, "MemTotal: ");
    k_itoa((int)total, tmp); k_strcat(buf, tmp); k_strcat(buf, " kB\n");
    k_strcat(buf, "MemFree: ");
    k_itoa((int)free, tmp);  k_strcat(buf, tmp); k_strcat(buf, " kB\n");
    (void)max;
}

static int proc_open(const char *path, int flags) {
    (void)flags;
    for (int i = 0; i < MAX_PROC_FDS; i++) {
        if (!g_pfds[i].len && !g_pfds[i].off && !g_pfds[i].buf[0]) {
            if (k_strcmp(path, "/version") == 0 || k_strcmp(path, "version") == 0)
                build_version(g_pfds[i].buf, sizeof(g_pfds[i].buf));
            else if (k_strcmp(path, "/meminfo") == 0 || k_strcmp(path, "meminfo") == 0)
                build_meminfo(g_pfds[i].buf, sizeof(g_pfds[i].buf));
            else return -1;
            g_pfds[i].len = (uint32_t)k_strlen(g_pfds[i].buf);
            g_pfds[i].off = 0;
            return i;
        }
    }
    return -1;
}

static int proc_read(int fd, void *buf, uint32_t len) {
    if (fd < 0 || fd >= MAX_PROC_FDS) return -1;
    proc_fd_t *p = &g_pfds[fd];
    uint32_t rem = p->len - p->off;
    if (len > rem) len = rem;
    k_memcpy(buf, p->buf + p->off, len);
    p->off += len;
    return (int)len;
}

static void proc_close(int fd) {
    if (fd >= 0 && fd < MAX_PROC_FDS) {
        g_pfds[fd].len = 0;
        g_pfds[fd].off = 0;
        g_pfds[fd].buf[0] = 0;
    }
}

static int proc_seek(int fd, uint32_t off) {
    if (fd >= 0 && fd < MAX_PROC_FDS) { g_pfds[fd].off = off; return 0; }
    return -1;
}

static int proc_exists(const char *path) {
    return (k_strcmp(path, "/version") == 0 || k_strcmp(path, "version") == 0 ||
            k_strcmp(path, "/meminfo") == 0 || k_strcmp(path, "meminfo") == 0) ? 1 : 0;
}

static const vfs_ops_t g_procfs_ops = {
    .open = proc_open, .read = proc_read, .write = 0,
    .close = proc_close, .seek = proc_seek,
    .list = 0, .mkdir = 0, .unlink = 0,
    .exists = proc_exists, .finfo = 0,
};

const vfs_ops_t *procfs_ops(void) { return &g_procfs_ops; }
