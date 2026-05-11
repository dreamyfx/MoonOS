// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "tty.h"
#include "../lib/kstring.h"
#include "../lib/console.h"
#include "../cpu/spinlock.h"

typedef struct {
    int  active;
    int  fg_pid;
    char in_buf[TTY_BUF];
    int  in_head, in_tail;
    char out_buf[TTY_BUF];
    int  out_head, out_tail;
} tty_t;

static tty_t g_ttys[MAX_TTYS];
static spinlock_t g_lock = SPINLOCK_INIT;

void tty_init(void) {
    k_memset(g_ttys, 0, sizeof(g_ttys));
    g_ttys[0].active = 1;
}

int tty_create(void) {
    uint64_t fl = spinlock_acquire_irqsave(&g_lock);
    for (int i = 1; i < MAX_TTYS; i++) {
        if (!g_ttys[i].active) {
            g_ttys[i].active = 1;
            g_ttys[i].fg_pid = 0;
            g_ttys[i].in_head = g_ttys[i].in_tail = 0;
            g_ttys[i].out_head = g_ttys[i].out_tail = 0;
            spinlock_release_irqrestore(&g_lock, fl);
            return i;
        }
    }
    spinlock_release_irqrestore(&g_lock, fl);
    return -1;
}

void tty_destroy(int id) {
    if (id < 0 || id >= MAX_TTYS) return;
    g_ttys[id].active = 0;
}

void tty_write_out(int id, const char *buf, int len) {
    if (id < 0 || id >= MAX_TTYS || !g_ttys[id].active) return;
    for (int i = 0; i < len; i++) {
        console_putc(buf[i]);
    }
}

int tty_read_out(int id, char *buf, int len) {
    (void)id; (void)buf; (void)len;
    return 0;
}

void tty_write_in(int id, const char *buf, int len) {
    if (id < 0 || id >= MAX_TTYS) return;
    tty_t *t = &g_ttys[id];
    for (int i = 0; i < len; i++) {
        int next = (t->in_tail + 1) % TTY_BUF;
        if (next != t->in_head) {
            t->in_buf[t->in_tail] = buf[i];
            t->in_tail = next;
        }
    }
}

int tty_read_in(int id, char *buf, int len) {
    if (id < 0 || id >= MAX_TTYS) return 0;
    tty_t *t = &g_ttys[id];
    int n = 0;
    while (n < len && t->in_head != t->in_tail) {
        buf[n++] = t->in_buf[t->in_head];
        t->in_head = (t->in_head + 1) % TTY_BUF;
    }
    return n;
}

void tty_set_fg(int id, int pid) {
    if (id >= 0 && id < MAX_TTYS) g_ttys[id].fg_pid = pid;
}

int tty_get_fg(int id) {
    if (id < 0 || id >= MAX_TTYS) return 0;
    return g_ttys[id].fg_pid;
}

void tty_kill_fg(int id) {
    if (id < 0 || id >= MAX_TTYS) return;
    g_ttys[id].fg_pid = 0;
}

static char g_input_buf[TTY_BUF];
static int  g_input_head, g_input_tail;

void tty_input_char(char c) {
    int next = (g_input_tail + 1) % TTY_BUF;
    if (next != g_input_head) {
        g_input_buf[g_input_tail] = c;
        g_input_tail = next;
    }
    tty_write_in(0, &c, 1);
}

int tty_current_input(char *buf, int len) {
    int n = 0;
    while (n < len && g_input_head != g_input_tail) {
        buf[n++] = g_input_buf[g_input_head];
        g_input_head = (g_input_head + 1) % TTY_BUF;
    }
    return n;
}
