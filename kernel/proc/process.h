// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../mm/vmm.h"

#define MAX_FDS   64
#define MAX_PROCS 64
#define USER_STACK_TOP 0x7FFFFFFFE000ULL
#define USER_STACK_SIZE (1024 * 1024)

typedef enum { PROC_DEAD=0, PROC_READY, PROC_RUNNING, PROC_BLOCKED, PROC_ZOMBIE } proc_state_t;

typedef struct {
    int      used;
    int      vfs_fd;
    uint64_t offset;
} fd_entry_t;

typedef struct process {
    uint32_t     pid;
    uint32_t     ppid;
    proc_state_t state;
    pagemap_t    pagemap;
    uint64_t     rsp;
    uint64_t     kernel_stack;
    uint64_t     heap_base;
    uint64_t     heap_top;
    char         name[64];
    char         cwd[256];
    fd_entry_t   fds[MAX_FDS];
    int          exit_code;
    int          tty_id;
    uint32_t     text_color;
} process_t;

void      proc_init(void);
process_t *proc_spawn(const char *path, const char *args, int tty_id);
process_t *proc_current(void);
void      proc_exit(int code);
void      proc_yield(void);
void      proc_sleep(uint32_t ms);
void      proc_schedule(void);
process_t *proc_get(uint32_t pid);
int       proc_waitpid(uint32_t pid, int *status);
void      proc_kill(uint32_t pid);
int       proc_alloc_fd(process_t *p, int vfs_fd);
void      proc_free_fd(process_t *p, int fd);
int       proc_get_vfs_fd(process_t *p, int fd);
void      proc_set_fd_offset(process_t *p, int fd, uint64_t off);
uint64_t  proc_get_fd_offset(process_t *p, int fd);
