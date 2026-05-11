// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "tarfs.h"
#include "../lib/kstring.h"
#include "../mm/heap.h"

#define TAR_MAX_FILES 512

typedef struct __attribute__((packed)) {
    char name[100], mode[8], uid[8], gid[8], size[12], mtime[12];
    char checksum[8], type;
    char linkname[100], magic[6], version[2], uname[32], gname[32];
    char devmajor[8], devminor[8], prefix[155];
    char padding[12];
} tar_header_t;

typedef struct {
    char     path[256];
    uint8_t *data;
    uint32_t size;
    uint8_t  is_dir;
} tar_entry_t;

static tar_entry_t g_entries[TAR_MAX_FILES];
static int         g_nentries;

typedef struct { int idx; uint32_t off; } tar_fd_t;
#define MAX_TAR_FDS 64
static tar_fd_t g_fds[MAX_TAR_FDS];

static uint32_t oct2int(const char *s, int n) {
    uint32_t v = 0;
    for (int i = 0; i < n && s[i] >= '0' && s[i] <= '7'; i++)
        v = v * 8 + (s[i] - '0');
    return v;
}

void tarfs_init(void *data, uint64_t size) {
    uint8_t *p = data;
    uint64_t off = 0;
    g_nentries = 0;
    while (off + 512 <= size) {
        tar_header_t *h = (tar_header_t *)(p + off);
        if (!h->name[0]) break;
        uint32_t fsize = oct2int(h->size, 11);
        if (g_nentries < TAR_MAX_FILES) {
            tar_entry_t *e = &g_entries[g_nentries++];
            k_strncpy(e->path, h->name, 255);
            e->data   = p + off + 512;
            e->size   = fsize;
            e->is_dir = (h->type == '5' || (fsize == 0 && h->name[k_strlen(h->name)-1] == '/'));
        }
        off += 512 + ((fsize + 511) & ~511ULL);
    }
}

static int find_entry(const char *path) {
    for (int i = 0; i < g_nentries; i++) {
        const char *ep = g_entries[i].path;
        if (ep[0] == '.' && ep[1] == '/') ep += 2;
        const char *rp = path;
        if (rp[0] == '/') rp++;
        if (k_strcmp(ep, rp) == 0) return i;
    }
    return -1;
}

static int tar_open(const char *path, int flags) {
    (void)flags;
    int idx = find_entry(path);
    if (idx < 0) return -1;
    for (int i = 0; i < MAX_TAR_FDS; i++) {
        if (g_fds[i].idx == -1 || (g_fds[i].idx == 0 && i > 0)) {
            g_fds[i].idx = idx;
            g_fds[i].off = 0;
            return i;
        }
    }
    return -1;
}

static int tar_read(int fd, void *buf, uint32_t len) {
    if (fd < 0 || fd >= MAX_TAR_FDS) return -1;
    tar_entry_t *e = &g_entries[g_fds[fd].idx];
    uint32_t rem = e->size - g_fds[fd].off;
    if (len > rem) len = rem;
    k_memcpy(buf, e->data + g_fds[fd].off, len);
    g_fds[fd].off += len;
    return (int)len;
}

static void tar_close(int fd) {
    if (fd >= 0 && fd < MAX_TAR_FDS) g_fds[fd].idx = -1;
}

static int tar_seek(int fd, uint32_t off) {
    if (fd < 0 || fd >= MAX_TAR_FDS) return -1;
    g_fds[fd].off = off;
    return 0;
}

static int tar_list(const char *path, vfs_dirent_t *out, int max) {
    const char *rp = path;
    if (rp[0] == '/') rp++;
    int n = 0;
    for (int i = 0; i < g_nentries && n < max; i++) {
        const char *ep = g_entries[i].path;
        if (ep[0] == '.' && ep[1] == '/') ep += 2;
        if (*rp == 0) {
            const char *slash = k_strchr(ep, '/');
            if (!slash || slash[1] == 0) {
                k_strncpy(out[n].name, ep, 255);
                if (slash) out[n].name[slash - ep] = 0;
                out[n].size   = g_entries[i].size;
                out[n].is_dir = g_entries[i].is_dir;
                n++;
            }
        } else {
            size_t plen = k_strlen(rp);
            if (k_strncmp(ep, rp, plen) == 0 && ep[plen] == '/') {
                const char *rest = ep + plen + 1;
                if (*rest && !k_strchr(rest, '/')) {
                    k_strncpy(out[n].name, rest, 255);
                    out[n].size   = g_entries[i].size;
                    out[n].is_dir = g_entries[i].is_dir;
                    n++;
                }
            }
        }
    }
    return n;
}

static int tar_exists(const char *path) { return find_entry(path) >= 0 ? 1 : 0; }

static int tar_finfo(const char *path, vfs_dirent_t *out) {
    int idx = find_entry(path);
    if (idx < 0) return -1;
    k_strncpy(out->name, g_entries[idx].path, 255);
    out->size   = g_entries[idx].size;
    out->is_dir = g_entries[idx].is_dir;
    return 0;
}

static const vfs_ops_t g_tar_ops = {
    .open   = tar_open,
    .read   = tar_read,
    .write  = 0,
    .close  = tar_close,
    .seek   = tar_seek,
    .list   = tar_list,
    .mkdir  = 0,
    .unlink = 0,
    .exists = tar_exists,
    .finfo  = tar_finfo,
};

const vfs_ops_t *tarfs_ops(void) {
    for (int i = 0; i < MAX_TAR_FDS; i++) g_fds[i].idx = -1;
    return &g_tar_ops;
}
