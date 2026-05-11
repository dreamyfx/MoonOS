// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stddef.h>

#define VFS_MAX_FDS 256

typedef struct {
    char     name[256];
    uint32_t size;
    uint8_t  is_dir;
} vfs_dirent_t;

typedef struct vfs_ops {
    int  (*open)(const char *path, int flags);
    int  (*read)(int fd, void *buf, uint32_t len);
    int  (*write)(int fd, const void *buf, uint32_t len);
    void (*close)(int fd);
    int  (*seek)(int fd, uint32_t off);
    int  (*list)(const char *path, vfs_dirent_t *out, int max);
    int  (*mkdir)(const char *path);
    int  (*unlink)(const char *path);
    int  (*exists)(const char *path);
    int  (*finfo)(const char *path, vfs_dirent_t *out);
} vfs_ops_t;

void vfs_init(void);
void vfs_mount(const char *mnt, const vfs_ops_t *ops);
int  vfs_open(const char *path, int flags);
int  vfs_read(int fd, void *buf, uint32_t len);
int  vfs_write(int fd, const void *buf, uint32_t len);
void vfs_close(int fd);
int  vfs_seek(int fd, uint32_t off);
int  vfs_list(const char *path, vfs_dirent_t *out, int max);
int  vfs_mkdir(const char *path);
int  vfs_unlink(const char *path);
int  vfs_exists(const char *path);
int  vfs_finfo(const char *path, vfs_dirent_t *out);
