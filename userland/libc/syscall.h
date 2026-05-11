// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "syscall_nums.h"

extern uint64_t syscall0(uint64_t n);
extern uint64_t syscall1(uint64_t n, uint64_t a);
extern uint64_t syscall2(uint64_t n, uint64_t a, uint64_t b);
extern uint64_t syscall3(uint64_t n, uint64_t a, uint64_t b, uint64_t c);
extern uint64_t syscall4(uint64_t n, uint64_t a, uint64_t b, uint64_t c, uint64_t d);

typedef struct {
    char name[256];
    uint32_t size;
    uint8_t  is_dir;
} dirent_t;

typedef struct {
    char os_name[64], os_version[64], os_codename[64];
    char kernel_name[64], kernel_version[64];
    char build_date[64], build_time[64], build_arch[64];
} os_info_t;

static inline void     sys_exit(int c)                       { syscall1(SYS_EXIT, (uint64_t)c); }
static inline int      sys_write(int fd, const void *b, int l) { return (int)syscall3(SYS_WRITE,(uint64_t)fd,(uint64_t)b,(uint64_t)l); }
static inline int      sys_read(int fd, void *b, int l)      { return (int)syscall3(SYS_READ,(uint64_t)fd,(uint64_t)b,(uint64_t)l); }
static inline int      sys_open(const char *p, int f)        { return (int)syscall2(SYS_OPEN,(uint64_t)p,(uint64_t)f); }
static inline void     sys_close(int fd)                     { syscall1(SYS_CLOSE,(uint64_t)fd); }
static inline void    *sys_sbrk(int n)                       { return (void*)syscall1(SYS_SBRK,(uint64_t)n); }
static inline int      sys_getcwd(char *b, int n)            { return (int)syscall2(SYS_GETCWD,(uint64_t)b,(uint64_t)n); }
static inline int      sys_chdir(const char *p)              { return (int)syscall1(SYS_CHDIR,(uint64_t)p); }
static inline int      sys_exists(const char *p)             { return (int)syscall1(SYS_EXISTS,(uint64_t)p); }
static inline int      sys_list(const char *p, dirent_t *o, int m) { return (int)syscall3(SYS_LIST,(uint64_t)p,(uint64_t)o,(uint64_t)m); }
static inline int      sys_finfo(const char *p, dirent_t *o) { return (int)syscall2(SYS_FINFO,(uint64_t)p,(uint64_t)o); }
static inline int      sys_mkdir(const char *p)              { return (int)syscall1(SYS_MKDIR,(uint64_t)p); }
static inline int      sys_unlink(const char *p)             { return (int)syscall1(SYS_UNLINK,(uint64_t)p); }
static inline int      sys_spawn(const char *p, const char *a, uint64_t f, uint64_t t) { return (int)syscall4(SYS_SPAWN,(uint64_t)p,(uint64_t)a,f,t); }
static inline int      sys_waitpid(int pid, int *s, int o)   { return (int)syscall3(SYS_WAITPID,(uint64_t)pid,(uint64_t)s,(uint64_t)o); }
static inline void     sys_reboot(void)                      { syscall0(SYS_REBOOT); }
static inline void     sys_halt(void)                        { syscall0(SYS_HALT); }
static inline void     sys_rtc_get(int dt[6])                { syscall1(SYS_RTC_GET,(uint64_t)dt); }
static inline void     sys_set_color(uint32_t c)             { syscall1(SYS_SET_COLOR,(uint64_t)c); }
static inline int      sys_tty_read_in(char *b, int l)       { return (int)syscall2(SYS_TTY_READ_IN,(uint64_t)b,(uint64_t)l); }
static inline void     sys_sleep(int ms)                     { syscall1(SYS_SLEEP,(uint64_t)ms); }
static inline int      sys_get_os_info(os_info_t *o)         { return (int)syscall1(SYS_GET_OS_INFO,(uint64_t)o); }
static inline void     sys_tty_set_fg(int t, int p)          { syscall2(SYS_TTY_SET_FG,(uint64_t)t,(uint64_t)p); }
static inline int      sys_tty_get_fg(int t)                 { return (int)syscall1(SYS_TTY_GET_FG,(uint64_t)t); }
static inline int      sys_seek(int fd, int off)             { return (int)syscall2(SYS_SEEK,(uint64_t)fd,(uint64_t)off); }
