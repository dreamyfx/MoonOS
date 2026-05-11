// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>

#define MAX_TTYS 8
#define TTY_BUF  4096

void tty_init(void);
int  tty_create(void);
void tty_destroy(int id);
void tty_write_out(int id, const char *buf, int len);
int  tty_read_out(int id, char *buf, int len);
void tty_write_in(int id, const char *buf, int len);
int  tty_read_in(int id, char *buf, int len);
void tty_set_fg(int id, int pid);
int  tty_get_fg(int id);
void tty_kill_fg(int id);
void tty_input_char(char c);
int  tty_current_input(char *buf, int len);
