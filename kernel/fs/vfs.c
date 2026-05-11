// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "vfs.h"
#include "../lib/kstring.h"
#include "../mm/heap.h"

#define MAX_MOUNTS 16

typedef struct {
    char       path[128];
    int        len;
    const vfs_ops_t *ops;
} mount_t;

static mount_t g_mounts[MAX_MOUNTS];
static int     g_nmounts;

typedef struct { const vfs_ops_t *ops; int inner_fd; } fd_slot_t;
static fd_slot_t g_fds[VFS_MAX_FDS];

void vfs_init(void) {
    k_memset(g_mounts, 0, sizeof(g_mounts));
    k_memset(g_fds,    0, sizeof(g_fds));
    g_nmounts = 0;
}

void vfs_mount(const char *mnt, const vfs_ops_t *ops) {
    if (g_nmounts >= MAX_MOUNTS) return;
    k_strncpy(g_mounts[g_nmounts].path, mnt, 127);
    g_mounts[g_nmounts].len = (int)k_strlen(mnt);
    g_mounts[g_nmounts].ops = ops;
    g_nmounts++;
}

static const vfs_ops_t *resolve(const char *path, const char **rel) {
    int best = -1, blen = -1;
    for (int i = 0; i < g_nmounts; i++) {
        if (k_starts_with(path, g_mounts[i].path) && g_mounts[i].len > blen) {
            best = i; blen = g_mounts[i].len;
        }
    }
    if (best < 0) return 0;
    const char *r = path + g_mounts[best].len;
    if (!*r) r = "/";
    *rel = r;
    return g_mounts[best].ops;
}

static int alloc_fd(const vfs_ops_t *ops, int inner) {
    for (int i = 0; i < VFS_MAX_FDS; i++) {
        if (!g_fds[i].ops) {
            g_fds[i].ops = ops;
            g_fds[i].inner_fd = inner;
            return i;
        }
    }
    return -1;
}

int vfs_open(const char *path, int flags) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->open) return -1;
    int inner = ops->open(rel, flags);
    if (inner < 0) return -1;
    return alloc_fd(ops, inner);
}

int vfs_read(int fd, void *buf, uint32_t len) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_fds[fd].ops) return -1;
    if (!g_fds[fd].ops->read) return -1;
    return g_fds[fd].ops->read(g_fds[fd].inner_fd, buf, len);
}

int vfs_write(int fd, const void *buf, uint32_t len) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_fds[fd].ops) return -1;
    if (!g_fds[fd].ops->write) return -1;
    return g_fds[fd].ops->write(g_fds[fd].inner_fd, buf, len);
}

void vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_fds[fd].ops) return;
    if (g_fds[fd].ops->close) g_fds[fd].ops->close(g_fds[fd].inner_fd);
    g_fds[fd].ops = 0;
}

int vfs_seek(int fd, uint32_t off) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_fds[fd].ops) return -1;
    if (!g_fds[fd].ops->seek) return -1;
    return g_fds[fd].ops->seek(g_fds[fd].inner_fd, off);
}

int vfs_list(const char *path, vfs_dirent_t *out, int max) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->list) return -1;
    return ops->list(rel, out, max);
}

int vfs_mkdir(const char *path) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->mkdir) return -1;
    return ops->mkdir(rel);
}

int vfs_unlink(const char *path) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->unlink) return -1;
    return ops->unlink(rel);
}

int vfs_exists(const char *path) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->exists) return 0;
    return ops->exists(rel);
}

int vfs_finfo(const char *path, vfs_dirent_t *out) {
    const char *rel;
    const vfs_ops_t *ops = resolve(path, &rel);
    if (!ops || !ops->finfo) return -1;
    return ops->finfo(rel, out);
}
