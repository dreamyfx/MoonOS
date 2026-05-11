// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stddef.h>
#include <stdbool.h>

size_t   k_strlen(const char *s);
int      k_strcmp(const char *a, const char *b);
int      k_strncmp(const char *a, const char *b, size_t n);
char    *k_strcpy(char *dst, const char *src);
char    *k_strncpy(char *dst, const char *src, size_t n);
char    *k_strcat(char *dst, const char *src);
char    *k_strchr(const char *s, int c);
char    *k_strstr(const char *hay, const char *needle);
void    *k_memcpy(void *dst, const void *src, size_t n);
void    *k_memset(void *dst, int c, size_t n);
int      k_memcmp(const void *a, const void *b, size_t n);
bool     k_starts_with(const char *s, const char *prefix);
void     k_itoa(int n, char *buf);
int      k_atoi(const char *s);
