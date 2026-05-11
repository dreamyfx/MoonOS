// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "kstring.h"

size_t k_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int k_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int k_strncmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        if (!a[i]) return 0;
    }
    return 0;
}

char *k_strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

char *k_strncpy(char *dst, const char *src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = 0;
    return dst;
}

char *k_strcat(char *dst, const char *src) {
    char *d = dst + k_strlen(dst);
    while ((*d++ = *src++));
    return dst;
}

char *k_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return (c == 0) ? (char *)s : 0;
}

char *k_strstr(const char *hay, const char *needle) {
    size_t nl = k_strlen(needle);
    if (!nl) return (char *)hay;
    for (; *hay; hay++) {
        if (k_strncmp(hay, needle, nl) == 0) return (char *)hay;
    }
    return 0;
}

void *k_memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

void *k_memset(void *dst, int c, size_t n) {
    uint8_t *d = dst;
    for (size_t i = 0; i < n; i++) d[i] = (uint8_t)c;
    return dst;
}

int k_memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *pa = a, *pb = b;
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}

bool k_starts_with(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return false;
    }
    return true;
}

void k_itoa(int n, char *buf) {
    if (n < 0) { *buf++ = '-'; n = -n; }
    char tmp[12]; int i = 0;
    if (n == 0) { *buf++ = '0'; *buf = 0; return; }
    while (n) { tmp[i++] = '0' + (n % 10); n /= 10; }
    while (i--) *buf++ = tmp[i];
    *buf = 0;
}

int k_atoi(const char *s) {
    int n = 0, neg = 0;
    while (*s == ' ') s++;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') n = n * 10 + (*s++ - '0');
    return neg ? -n : n;
}
