// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "syscall.h"
#include "sysnums.h"
#include "../proc/process.h"
#include "../fs/vfs.h"
#include "../tty/tty.h"
#include "../lib/kstring.h"
#include "../lib/console.h"
#include "../mm/pmm.h"
#include "../io/rtc.h"
#include <stdint.h>

typedef struct {
    uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
    uint64_t rdi,rsi,rbp,rbx,rdx,rcx,rax;
    uint64_t rip,cs,rflags,rsp,ss;
} sc_regs_t;

extern uint64_t g_hhdm_offset;

static const char *uptr(uint64_t addr) {
    if (!addr) return 0;
    process_t *p = proc_current();
    if (!p) return 0;
    extern uint64_t vmm_translate(uint64_t pm, uint64_t virt);
    uint64_t phys = vmm_translate(p->pagemap, addr & ~0xFFFULL);
    if (!phys) return (const char *)addr;
    return (const char *)(phys + g_hhdm_offset + (addr & 0xFFF));
}

static void *uptr_rw(uint64_t addr) { return (void *)uptr(addr); }

uint64_t syscall_dispatch(uint64_t num, void *regs_raw) {
    sc_regs_t *r = regs_raw;
    uint64_t a1 = r->rdi, a2 = r->rsi, a3 = r->rdx, a4 = r->r10;
    process_t *p = proc_current();

    switch (num) {
    case SYS_EXIT:
        proc_exit((int)a1);
        return 0;

    case SYS_WRITE: {
        int tty_id = p ? p->tty_id : 0;
        const char *buf = uptr(a2);
        if (!buf) return (uint64_t)-1;
        if ((int)a1 == 1 || (int)a1 == 2) {
            if (p) console_set_color(p->text_color);
            tty_write_out(tty_id, buf, (int)a3);
            return a3;
        }
        if (p) {
            int vfd = proc_get_vfs_fd(p, (int)a1);
            if (vfd >= 0) return (uint64_t)vfs_write(vfd, buf, (uint32_t)a3);
        }
        return (uint64_t)-1;
    }

    case SYS_READ: {
        if ((int)a1 == 0) {
            char *buf = uptr_rw(a2);
            if (!buf) return (uint64_t)-1;
            int tty_id = p ? p->tty_id : 0;
            int got = 0;
            while (got == 0) {
                got = tty_read_in(tty_id, buf, (int)a3);
                if (got == 0) proc_yield();
            }
            return (uint64_t)got;
        }
        if (p) {
            int vfd = proc_get_vfs_fd(p, (int)a1);
            if (vfd >= 0) {
                void *buf = uptr_rw(a2);
                return (uint64_t)vfs_read(vfd, buf, (uint32_t)a3);
            }
        }
        return (uint64_t)-1;
    }

    case SYS_OPEN: {
        if (!p) return (uint64_t)-1;
        const char *path = uptr(a1);
        if (!path) return (uint64_t)-1;
        int vfd = vfs_open(path, (int)a2);
        if (vfd < 0) return (uint64_t)-1;
        int fd = proc_alloc_fd(p, vfd);
        return (uint64_t)fd;
    }

    case SYS_CLOSE:
        if (p && (int)a1 >= 3) {
            int vfd = proc_get_vfs_fd(p, (int)a1);
            if (vfd >= 0) { vfs_close(vfd); proc_free_fd(p, (int)a1); }
        }
        return 0;

    case SYS_SBRK: {
        if (!p) return (uint64_t)-1;
        int64_t incr = (int64_t)a1;
        uint64_t old = p->heap_top;
        p->heap_top += incr;
        if (incr > 0) {
            extern void vmm_map(uint64_t pm, uint64_t virt, uint64_t phys, uint64_t flags);
            extern uint64_t pmm_alloc(void);
            uint64_t pg_start = (old + 0xFFF) & ~0xFFFULL;
            uint64_t pg_end   = (p->heap_top + 0xFFF) & ~0xFFFULL;
            for (uint64_t v = pg_start; v < pg_end; v += 4096) {
                uint64_t phys = pmm_alloc();
                vmm_map(p->pagemap, v, phys, 0x7);
            }
        }
        return old;
    }

    case SYS_EXISTS: {
        const char *path = uptr(a1);
        if (!path) return 0;
        return (uint64_t)vfs_exists(path);
    }

    case SYS_FINFO: {
        const char *path = uptr(a1);
        void *out = uptr_rw(a2);
        if (!path || !out) return (uint64_t)-1;
        return (uint64_t)vfs_finfo(path, (vfs_dirent_t *)out);
    }

    case SYS_LIST: {
        const char *path = uptr(a1);
        void *out = uptr_rw(a2);
        if (!path || !out) return (uint64_t)-1;
        return (uint64_t)vfs_list(path, (vfs_dirent_t *)out, (int)a3);
    }

    case SYS_MKDIR: {
        const char *path = uptr(a1);
        if (!path) return (uint64_t)-1;
        return (uint64_t)vfs_mkdir(path);
    }

    case SYS_UNLINK: {
        const char *path = uptr(a1);
        if (!path) return (uint64_t)-1;
        return (uint64_t)vfs_unlink(path);
    }

    case SYS_GETCWD: {
        if (!p) return (uint64_t)-1;
        char *buf = uptr_rw(a1);
        if (!buf) return (uint64_t)-1;
        k_strncpy(buf, p->cwd, (int)a2);
        return 0;
    }

    case SYS_CHDIR: {
        const char *path = uptr(a1);
        if (!p || !path) return (uint64_t)-1;
        if (!vfs_exists(path)) return (uint64_t)-1;
        k_strncpy(p->cwd, path, 255);
        return 0;
    }

    case SYS_SPAWN: {
        const char *path = uptr(a1);
        const char *args = uptr(a2);
        if (!path) return (uint64_t)-1;
        int tty_id = p ? p->tty_id : 0;
        process_t *np = proc_spawn(path, args, tty_id);
        if (!np) return (uint64_t)-1;
        np->state = PROC_READY;
        return (uint64_t)np->pid;
    }

    case SYS_WAITPID: {
        int *status = uptr_rw(a2);
        return (uint64_t)proc_waitpid((uint32_t)a1, status);
    }

    case SYS_KILL:
        proc_kill((uint32_t)a1);
        return 0;

    case SYS_GETPID:
        return p ? p->pid : 0;

    case SYS_SEEK: {
        if (!p) return (uint64_t)-1;
        int vfd = proc_get_vfs_fd(p, (int)a1);
        if (vfd < 0) return (uint64_t)-1;
        return (uint64_t)vfs_seek(vfd, (uint32_t)a2);
    }

    case SYS_REBOOT:
        outb(0x64, 0xFE);
        for (;;) asm volatile("hlt");
        return 0;

    case SYS_HALT:
        asm volatile("cli; hlt");
        return 0;

    case SYS_RTC_GET: {
        int *dt = uptr_rw(a1);
        if (!dt) return (uint64_t)-1;
        rtc_get(dt);
        return 0;
    }

    case SYS_SET_COLOR:
        if (p) p->text_color = (uint32_t)a1;
        return 0;

    case SYS_TTY_CREATE:
        return (uint64_t)tty_create();

    case SYS_TTY_READ_IN: {
        char *buf = uptr_rw(a1);
        if (!buf) return (uint64_t)-1;
        int tty_id = p ? p->tty_id : 0;
        return (uint64_t)tty_read_in(tty_id, buf, (int)a2);
    }

    case SYS_TTY_SET_FG:
        tty_set_fg((int)a1, (int)a2);
        return 0;

    case SYS_TTY_GET_FG:
        return (uint64_t)tty_get_fg((int)a1);

    case SYS_TTY_DESTROY:
        tty_destroy((int)a1);
        return 0;

    case SYS_TTY_KILL_FG:
        tty_kill_fg((int)a1);
        return 0;

    case SYS_SLEEP:
        proc_sleep((uint32_t)a1);
        return 0;

    case SYS_SHELL_CFG:
        return 0;

    case SYS_WRITE_FS: {
        if (!p) return (uint64_t)-1;
        int vfd = proc_get_vfs_fd(p, (int)a1);
        const char *buf = uptr(a2);
        if (vfd < 0 || !buf) return (uint64_t)-1;
        return (uint64_t)vfs_write(vfd, buf, (uint32_t)a3);
    }

    case SYS_GET_OS_INFO: {
        void *out = uptr_rw(a1);
        if (!out) return (uint64_t)-1;
        typedef struct {
            char os_name[64], os_version[64], os_codename[64];
            char kernel_name[64], kernel_version[64];
            char build_date[64], build_time[64], build_arch[64];
        } os_info_t;
        os_info_t *info = out;
        k_strncpy(info->os_name,       "MoonOS",          63);
        k_strncpy(info->os_version,    "1.0.0",           63);
        k_strncpy(info->os_codename,   "Lunar",           63);
        k_strncpy(info->kernel_name,   "MoonKernel",      63);
        k_strncpy(info->kernel_version,"1.0.0-release",   63);
        k_strncpy(info->build_date,    __DATE__,          63);
        k_strncpy(info->build_time,    __TIME__,          63);
        k_strncpy(info->build_arch,    "x86_64",          63);
        return 0;
    }

    default:
        return (uint64_t)-1;
    }
}

void irq_dispatch(uint8_t irq) {
    extern void ps2_irq(void);
    extern void lapic_timer_tick(void);
    if (irq == 1) ps2_irq();
    if (irq == 0) { lapic_timer_tick(); proc_schedule(); }
}

void syscall_init(void) {
    uint64_t star  = ((uint64_t)0x13 << 48) | ((uint64_t)0x08 << 32);
    uint64_t lstar = (uint64_t)syscall_entry;
    uint64_t sfmask = (1 << 9);
    asm volatile("wrmsr" : : "c"(0xC0000080UL),
        "A"((uint64_t)1 | (rdmsr(0xC0000080UL) & ~1ULL)));
    asm volatile("wrmsr" : : "c"(0xC0000081UL), "A"(star));
    asm volatile("wrmsr" : : "c"(0xC0000082UL), "A"(lstar));
    asm volatile("wrmsr" : : "c"(0xC0000084UL), "A"(sfmask));
}

static uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

extern void syscall_entry(void);
