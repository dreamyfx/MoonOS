// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "stdlib.h"
#include <stdarg.h>

static char  *g_heap_cur;
static char  *g_heap_end;
static int    g_heap_init = 0;

static void heap_ensure(void) {
    if (!g_heap_init) {
        g_heap_cur = (char *)sys_sbrk(65536);
        g_heap_end = g_heap_cur + 65536;
        g_heap_init = 1;
    }
}

typedef struct { size_t size; int free; } hdr_t;

void *malloc(size_t n) {
    heap_ensure();
    n = (n + 15) & ~15ULL;
    char *p = g_heap_cur;
    if (p + sizeof(hdr_t) + n > g_heap_end) {
        char *more = (char *)sys_sbrk((int)(n + sizeof(hdr_t) + 4096));
        if (!more) return 0;
        g_heap_end = more + n + sizeof(hdr_t) + 4096;
    }
    hdr_t *h = (hdr_t *)g_heap_cur;
    h->size = n; h->free = 0;
    g_heap_cur += sizeof(hdr_t) + n;
    return (char *)h + sizeof(hdr_t);
}

void free(void *p) { if (p) { hdr_t *h = (hdr_t *)((char*)p - sizeof(hdr_t)); h->free = 1; } }

void *calloc(size_t n, size_t s) {
    void *p = malloc(n * s);
    if (p) memset(p, 0, n * s);
    return p;
}

void *realloc(void *p, size_t n) {
    if (!p) return malloc(n);
    void *np = malloc(n);
    if (np) { hdr_t *h = (hdr_t *)((char*)p - sizeof(hdr_t)); memcpy(np, p, h->size < n ? h->size : n); free(p); }
    return np;
}

void *memset(void *d, int c, size_t n) {
    uint8_t *p = d;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return d;
}

void *memcpy(void *d, const void *s, size_t n) {
    uint8_t *pd = d; const uint8_t *ps = s;
    for (size_t i = 0; i < n; i++) pd[i] = ps[i];
    return d;
}

int memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *pa = a, *pb = b;
    for (size_t i = 0; i < n; i++) if (pa[i] != pb[i]) return pa[i] - pb[i];
    return 0;
}

int strlen(const char *s) { int n = 0; while (s[n]) n++; return n; }

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        if (!a[i]) return 0;
    }
    return 0;
}

char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)); return r; }

char *strncpy(char *d, const char *s, int n) {
    int i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
    return d;
}

char *strcat(char *d, const char *s) {
    char *r = d + strlen(d);
    while ((*r++ = *s++));
    return d;
}

char *strchr(const char *s, int c) {
    while (*s) { if (*s == (char)c) return (char*)s; s++; }
    return c == 0 ? (char*)s : 0;
}

char *strstr(const char *h, const char *n) {
    int nl = strlen(n);
    if (!nl) return (char*)h;
    for (; *h; h++) if (strncmp(h, n, nl) == 0) return (char*)h;
    return 0;
}

int atoi(const char *s) {
    int n = 0, neg = 0;
    while (*s == ' ') s++;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') n = n * 10 + (*s++ - '0');
    return neg ? -n : n;
}

void itoa(int n, char *buf) {
    if (n < 0) { *buf++ = '-'; n = -n; }
    char tmp[12]; int i = 0;
    if (!n) { *buf++ = '0'; *buf = 0; return; }
    while (n) { tmp[i++] = '0' + n % 10; n /= 10; }
    while (i--) *buf++ = tmp[i];
    *buf = 0;
}

void exit(int c) { sys_exit(c); for(;;); }
char *getcwd(char *buf, int n) { sys_getcwd(buf, n); return buf; }
int   chdir(const char *p) { return sys_chdir(p); }
void  sleep(int ms) { sys_sleep(ms); }
void  puts(const char *s) { sys_write(1, s, strlen(s)); sys_write(1, "\n", 1); }

void printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    char buf[1024]; int pos = 0;
    for (const char *p = fmt; *p && pos < 1023; p++) {
        if (*p != '%') { buf[pos++] = *p; continue; }
        p++;
        switch (*p) {
        case 's': { const char *s = va_arg(ap, const char*); if (!s) s = "(null)"; while (*s && pos < 1023) buf[pos++] = *s++; break; }
        case 'd': { int n = va_arg(ap, int); char tmp[16]; itoa(n, tmp); for (int i=0; tmp[i] && pos<1023; i++) buf[pos++]=tmp[i]; break; }
        case 'u': { unsigned n = va_arg(ap, unsigned); char tmp[16]; itoa((int)n, tmp); for (int i=0; tmp[i] && pos<1023; i++) buf[pos++]=tmp[i]; break; }
        case 'c': { buf[pos++] = (char)va_arg(ap, int); break; }
        case 'x': { unsigned long long n = va_arg(ap, unsigned long long); char tmp[32]; int i=15; tmp[16]=0; if(!n){buf[pos++]='0';break;} while(n){int d=n&0xF;tmp[i--]=d<10?'0'+d:'a'+d-10;n>>=4;} char *t=tmp+i+1; while(*t && pos<1023) buf[pos++]=*t++; break; }
        case '%': buf[pos++] = '%'; break;
        default: buf[pos++] = '%'; buf[pos++] = *p; break;
        }
    }
    buf[pos] = 0;
    va_end(ap);
    sys_write(1, buf, pos);
}
