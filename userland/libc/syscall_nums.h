// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once

#define SYS_EXIT        0
#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_OPEN        3
#define SYS_CLOSE       4
#define SYS_SBRK        5
#define SYS_LIST        6
#define SYS_MKDIR       7
#define SYS_UNLINK      8
#define SYS_GETCWD      9
#define SYS_CHDIR       10
#define SYS_FINFO       11
#define SYS_SPAWN       12
#define SYS_WAITPID     13
#define SYS_REBOOT      14
#define SYS_HALT        15
#define SYS_RTC_GET     16
#define SYS_SET_COLOR   17
#define SYS_TTY_CREATE  18
#define SYS_TTY_READ_IN 19
#define SYS_TTY_SET_FG  20
#define SYS_TTY_GET_FG  21
#define SYS_SLEEP       22
#define SYS_EXISTS      23
#define SYS_GET_OS_INFO 24
#define SYS_KILL        25
#define SYS_GETPID      26
#define SYS_SEEK        27
#define SYS_TELL        28
#define SYS_DUP2        29
#define SYS_TTY_DESTROY 30
#define SYS_TTY_KILL_FG 31
#define SYS_SHELL_CFG   32
#define SYS_WRITE_FS    33

#define SPAWN_TERMINAL    0x1
#define SPAWN_INHERIT_TTY 0x2
