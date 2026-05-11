// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "../libc/stdlib.h"
#include "../libc/syscall.h"

#define MAX_LINE 512
#define MAX_ARGS 32
#define MAX_HIST 128
#define MAX_PATHS 8
#define MAX_PATH_LEN 128

static char g_hist[MAX_HIST][MAX_LINE];
static int  g_hist_count;
static char g_paths[MAX_PATHS][MAX_PATH_LEN];
static int  g_path_count;

static void split_path(const char *path) {
    g_path_count = 0;
    int i = 0, start = 0;
    while (1) {
        if (path[i] == ':' || !path[i]) {
            int len = i - start;
            if (len > 0 && g_path_count < MAX_PATHS) {
                strncpy(g_paths[g_path_count], path + start, len < MAX_PATH_LEN-1 ? len : MAX_PATH_LEN-1);
                g_paths[g_path_count][len] = 0;
                g_path_count++;
            }
            start = i + 1;
        }
        if (!path[i]) break;
        i++;
    }
}

static void hist_add(const char *line) {
    if (!line[0]) return;
    if (g_hist_count > 0 && strcmp(g_hist[g_hist_count-1], line) == 0) return;
    if (g_hist_count < MAX_HIST) {
        strncpy(g_hist[g_hist_count++], line, MAX_LINE-1);
    } else {
        for (int i = 1; i < MAX_HIST; i++) memcpy(g_hist[i-1], g_hist[i], MAX_LINE);
        strncpy(g_hist[MAX_HIST-1], line, MAX_LINE-1);
    }
}

static int split_args(char *line, char *argv[], int max) {
    int argc = 0, i = 0;
    while (line[i] && argc < max) {
        while (line[i] == ' ' || line[i] == '\t') i++;
        if (!line[i]) break;
        if (line[i] == '"') {
            i++;
            argv[argc++] = &line[i];
            while (line[i] && line[i] != '"') i++;
            if (line[i]) { line[i] = 0; i++; }
        } else {
            argv[argc++] = &line[i];
            while (line[i] && line[i] != ' ' && line[i] != '\t') i++;
            if (line[i]) { line[i] = 0; i++; }
        }
    }
    return argc;
}

static int resolve_bin(const char *cmd, char *out) {
    if (strchr(cmd, '/')) {
        if (sys_exists(cmd)) { strcpy(out, cmd); return 1; }
        char tmp[256];
        strcpy(tmp, cmd); strcat(tmp, ".elf");
        if (sys_exists(tmp)) { strcpy(out, tmp); return 1; }
        return 0;
    }
    for (int i = 0; i < g_path_count; i++) {
        char tmp[256];
        strcpy(tmp, g_paths[i]);
        if (tmp[strlen(tmp)-1] != '/') strcat(tmp, "/");
        strcat(tmp, cmd);
        if (sys_exists(tmp)) { strcpy(out, tmp); return 1; }
        strcat(tmp, ".elf");
        if (sys_exists(tmp)) { strcpy(out, tmp); return 1; }
    }
    return 0;
}

static void emit_prompt(void) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    sys_set_color(0xFF88AAFF);
    sys_write(1, "root", 4);
    sys_set_color(0xFFCCCCCC);
    sys_write(1, "@moonos:", 8);
    sys_set_color(0xFF88FFAA);
    sys_write(1, cwd, strlen(cwd));
    sys_set_color(0xFFCCCCCC);
    sys_write(1, "$ ", 2);
}

static int read_line(char *out, int max) {
    int len = 0, cursor = 0;
    int hist_idx = g_hist_count;
    char saved[MAX_LINE];
    saved[0] = 0;
    out[0] = 0;

    while (1) {
        char ch = 0;
        while (!sys_tty_read_in(&ch, 1)) sys_sleep(5);

        if (ch == 3) { sys_write(1, "^C\n", 3); out[0] = 0; return 0; }
        if (ch == '\r' || ch == '\n') { sys_write(1, "\n", 1); out[len] = 0; return len; }

        if (ch == '\b' || ch == 127) {
            if (cursor > 0) {
                for (int i = cursor; i <= len; i++) out[i-1] = out[i];
                cursor--; len--;
                sys_write(1, "\r", 1);
                emit_prompt();
                sys_write(1, out, len);
                sys_write(1, " \b", 2);
            }
            continue;
        }

        if (ch == '\x11') {
            if (g_hist_count == 0) continue;
            if (hist_idx == g_hist_count) strncpy(saved, out, MAX_LINE-1);
            if (hist_idx > 0) hist_idx--;
            strncpy(out, g_hist[hist_idx], max-1);
            len = cursor = strlen(out);
            sys_write(1, "\r", 1); emit_prompt();
            sys_write(1, out, len); sys_write(1, "\x1b[K", 3);
            continue;
        }
        if (ch == '\x12') {
            if (g_hist_count == 0) continue;
            if (hist_idx < g_hist_count - 1) { hist_idx++; strncpy(out, g_hist[hist_idx], max-1); }
            else { hist_idx = g_hist_count; strncpy(out, saved, max-1); }
            len = cursor = strlen(out);
            sys_write(1, "\r", 1); emit_prompt();
            sys_write(1, out, len); sys_write(1, "\x1b[K", 3);
            continue;
        }

        if (ch >= 32 && len < max - 1) {
            for (int i = len; i >= cursor; i--) out[i+1] = out[i];
            out[cursor++] = ch; len++;
            sys_write(1, "\r", 1); emit_prompt();
            sys_write(1, out, len);
            if (cursor < len) {
                char seq[16]; int diff = len - cursor;
                strcpy(seq, "\x1b["); char tmp[8]; itoa(diff, tmp);
                strcat(seq, tmp); strcat(seq, "D");
                sys_write(1, seq, strlen(seq));
            }
        }
    }
}

static int builtin_cd(int argc, char *argv[]) {
    const char *path = argc > 1 ? argv[1] : "/";
    if (chdir(path) != 0) {
        sys_set_color(0xFFFF4444);
        printf("cd: no such directory: %s\n", path);
        sys_set_color(0xFFCCCCCC);
        return 1;
    }
    return 0;
}

static int builtin_ls(int argc, char *argv[]) {
    char path[256];
    if (argc > 1) strncpy(path, argv[1], 255);
    else getcwd(path, sizeof(path));

    dirent_t entries[128];
    int n = sys_list(path, entries, 128);
    if (n < 0) {
        sys_set_color(0xFFFF4444);
        printf("ls: cannot list %s\n", path);
        sys_set_color(0xFFCCCCCC);
        return 1;
    }
    for (int i = 0; i < n; i++) {
        if (entries[i].is_dir) {
            sys_set_color(0xFF88AAFF);
            printf("%s/\n", entries[i].name);
        } else {
            sys_set_color(0xFFCCCCCC);
            printf("%s\n", entries[i].name);
        }
    }
    sys_set_color(0xFFCCCCCC);
    return 0;
}

static int builtin_cat(int argc, char *argv[]) {
    if (argc < 2) { printf("Usage: cat <file>\n"); return 1; }
    int fd = sys_open(argv[1], 0);
    if (fd < 0) {
        sys_set_color(0xFFFF4444);
        printf("cat: cannot open %s\n", argv[1]);
        sys_set_color(0xFFCCCCCC);
        return 1;
    }
    char buf[512]; int n;
    while ((n = sys_read(fd, buf, sizeof(buf))) > 0)
        sys_write(1, buf, n);
    sys_close(fd);
    return 0;
}

static int builtin_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) sys_write(1, " ", 1);
        sys_write(1, argv[i], strlen(argv[i]));
    }
    sys_write(1, "\n", 1);
    return 0;
}

static int builtin_clear(void) {
    sys_write(1, "\x1b[2J\x1b[H", 7);
    return 0;
}

static int builtin_pwd(void) {
    char cwd[256]; getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 0;
}

static int execute(const char *line) {
    if (!line || !line[0]) return 0;
    char copy[MAX_LINE];
    strncpy(copy, line, MAX_LINE-1);
    char *argv[MAX_ARGS];
    int argc = split_args(copy, argv, MAX_ARGS);
    if (!argc) return 0;

    if (strcmp(argv[0], "cd")    == 0) return builtin_cd(argc, argv);
    if (strcmp(argv[0], "ls")    == 0) return builtin_ls(argc, argv);
    if (strcmp(argv[0], "cat")   == 0) return builtin_cat(argc, argv);
    if (strcmp(argv[0], "echo")  == 0) return builtin_echo(argc, argv);
    if (strcmp(argv[0], "clear") == 0) return builtin_clear();
    if (strcmp(argv[0], "pwd")   == 0) return builtin_pwd();
    if (strcmp(argv[0], "exit")  == 0) { sys_exit(0); return 0; }

    char full[256];
    if (!resolve_bin(argv[0], full)) {
        sys_set_color(0xFFFF4444);
        printf("msh: command not found: %s\n", argv[0]);
        sys_set_color(0xFFCCCCCC);
        return 127;
    }

    char args_buf[256]; args_buf[0] = 0;
    for (int i = 1; i < argc; i++) {
        if (i > 1) strcat(args_buf, " ");
        strcat(args_buf, argv[i]);
    }

    int pid = sys_spawn(full, args_buf[0] ? args_buf : 0, SPAWN_TERMINAL | SPAWN_INHERIT_TTY, 0);
    if (pid < 0) {
        sys_set_color(0xFFFF4444);
        printf("msh: failed to launch %s\n", full);
        sys_set_color(0xFFCCCCCC);
        return 1;
    }
    sys_waitpid(pid, 0, 1);
    return 0;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    split_path("/bin");

    sys_set_color(0xFF88AAFF);
    sys_write(1, "Welcome to MoonOS 1.0.0\n", 24);
    sys_set_color(0xFFAAAAAA);
    sys_write(1, "Type 'help' for available commands.\n\n", 37);
    sys_set_color(0xFFCCCCCC);

    while (1) {
        emit_prompt();
        char line[MAX_LINE];
        int len = read_line(line, sizeof(line));
        if (len <= 0) continue;
        hist_add(line);
        execute(line);
    }
    return 0;
}
