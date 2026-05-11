// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "process.h"
#include "elf.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../lib/kstring.h"
#include "../lib/panic.h"
#include "../cpu/spinlock.h"
#include "../cpu/lapic.h"
#include "../cpu/gdt.h"

static process_t g_procs[MAX_PROCS];
static int       g_current = -1;
static spinlock_t g_lock = SPINLOCK_INIT;
static uint32_t  g_next_pid = 1;
extern uint64_t  g_hhdm_offset;

extern void ctx_switch(uint64_t *old_rsp, uint64_t *new_rsp);
extern void jump_usermode(uint64_t entry, uint64_t stack);

void proc_init(void) {
    k_memset(g_procs, 0, sizeof(g_procs));
}

static int proc_alloc_slot(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (g_procs[i].state == PROC_DEAD) return i;
    }
    return -1;
}

static void setup_user_stack(process_t *p, const char *args) {
    uint64_t stack_pages = USER_STACK_SIZE / PAGE_SIZE;
    for (uint64_t i = 0; i < stack_pages; i++) {
        uint64_t phys = pmm_alloc();
        vmm_map(p->pagemap,
                USER_STACK_TOP - USER_STACK_SIZE + i * PAGE_SIZE,
                phys, VMM_PRESENT | VMM_WRITE | VMM_USER);
    }
    p->rsp = USER_STACK_TOP - 8;

    if (args && args[0]) {
        size_t alen = k_strlen(args) + 1;
        p->rsp -= alen;
        p->rsp &= ~0xFULL;
        uint64_t phys = vmm_translate(p->pagemap, p->rsp);
        uint8_t *dst = (uint8_t *)(phys + g_hhdm_offset);
        k_memcpy(dst, args, alen);
    }
}

process_t *proc_spawn(const char *path, const char *args, int tty_id) {
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    int slot = proc_alloc_slot();
    if (slot < 0) { spinlock_release_irqrestore(&g_lock, fl); return 0; }

    process_t *p = &g_procs[slot];
    k_memset(p, 0, sizeof(*p));
    p->pid    = g_next_pid++;
    p->ppid   = g_current >= 0 ? g_procs[g_current].pid : 0;
    p->state  = PROC_READY;
    p->tty_id = tty_id;
    p->text_color = 0xFFCCCCCC;
    k_strncpy(p->name, path, 63);
    k_strcpy(p->cwd, "/");

    p->pagemap = vmm_new();
    p->kernel_stack = (uint64_t)kmalloc(8192) + 8192;
    p->heap_base = 0x40000000ULL;
    p->heap_top  = p->heap_base;

    for (int i = 0; i < MAX_FDS; i++) p->fds[i].used = 0;

    uint64_t entry = elf_load(p->pagemap, path);
    if (!entry) {
        kfree((void *)(p->kernel_stack - 8192));
        vmm_destroy(p->pagemap);
        p->state = PROC_DEAD;
        spinlock_release_irqrestore(&g_lock, fl);
        return 0;
    }

    setup_user_stack(p, args);
    p->rsp = entry;

    spinlock_release_irqrestore(&g_lock, fl);
    return p;
}

process_t *proc_current(void) {
    if (g_current < 0) return 0;
    return &g_procs[g_current];
}

void proc_schedule(void) {
    int next = -1;
    for (int i = 0; i < MAX_PROCS; i++) {
        int idx = (g_current + 1 + i) % MAX_PROCS;
        if (g_procs[idx].state == PROC_READY) { next = idx; break; }
    }
    if (next < 0) return;
    int old = g_current;
    g_current = next;
    g_procs[next].state = PROC_RUNNING;
    if (old >= 0 && g_procs[old].state == PROC_RUNNING)
        g_procs[old].state = PROC_READY;
    gdt_set_tss_rsp0(g_procs[next].kernel_stack);
    vmm_switch(g_procs[next].pagemap);
    if (old >= 0) {
        ctx_switch(&g_procs[old].rsp, &g_procs[next].rsp);
    } else {
        uint64_t dummy;
        ctx_switch(&dummy, &g_procs[next].rsp);
    }
}

void proc_run_first(process_t *p) {
    int slot = (int)(p - g_procs);
    g_current = slot;
    p->state = PROC_RUNNING;
    gdt_set_tss_rsp0(p->kernel_stack);
    vmm_switch(p->pagemap);
    jump_usermode(p->rsp, USER_STACK_TOP - 8);
}

void proc_exit(int code) {
    if (g_current < 0) return;
    process_t *p = &g_procs[g_current];
    p->state     = PROC_ZOMBIE;
    p->exit_code = code;
    g_current    = -1;
    proc_schedule();
}

process_t *proc_get(uint32_t pid) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (g_procs[i].pid == pid && g_procs[i].state != PROC_DEAD) return &g_procs[i];
    }
    return 0;
}

int proc_waitpid(uint32_t pid, int *status) {
    process_t *p = proc_get(pid);
    if (!p) return -1;
    while (p->state != PROC_ZOMBIE) proc_yield();
    if (status) *status = p->exit_code;
    p->state = PROC_DEAD;
    return 0;
}

void proc_kill(uint32_t pid) {
    process_t *p = proc_get(pid);
    if (p) p->state = PROC_ZOMBIE;
}

void proc_yield(void) { proc_schedule(); }

void proc_sleep(uint32_t ms) {
    uint32_t start = lapic_get_ticks();
    while (lapic_get_ticks() - start < ms) proc_yield();
}

int proc_alloc_fd(process_t *p, int vfs_fd) {
    for (int i = 3; i < MAX_FDS; i++) {
        if (!p->fds[i].used) {
            p->fds[i].used   = 1;
            p->fds[i].vfs_fd = vfs_fd;
            p->fds[i].offset = 0;
            return i;
        }
    }
    return -1;
}

void proc_free_fd(process_t *p, int fd) {
    if (fd >= 0 && fd < MAX_FDS) p->fds[fd].used = 0;
}

int proc_get_vfs_fd(process_t *p, int fd) {
    if (fd < 0 || fd >= MAX_FDS || !p->fds[fd].used) return -1;
    return p->fds[fd].vfs_fd;
}

void proc_set_fd_offset(process_t *p, int fd, uint64_t off) {
    if (fd >= 0 && fd < MAX_FDS) p->fds[fd].offset = off;
}

uint64_t proc_get_fd_offset(process_t *p, int fd) {
    if (fd < 0 || fd >= MAX_FDS) return 0;
    return p->fds[fd].offset;
}
