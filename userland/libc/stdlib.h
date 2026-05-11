// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "syscall.h"

void  *malloc(size_t n);
void   free(void *p);
void  *calloc(size_t n, size_t s);
void  *realloc(void *p, size_t n);
void  *memset(void *d, int c, size_t n);
void  *memcpy(void *d, const void *s, size_t n);
int    memcmp(const void *a, const void *b, size_t n);
int    strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, int n);
char  *strcpy(char *d, const char *s);
char  *strncpy(char *d, const char *s, int n);
char  *strcat(char *d, const char *s);
char  *strchr(const char *s, int c);
char  *strstr(const char *h, const char *n);
int    atoi(const char *s);
void   itoa(int n, char *buf);
void   exit(int c);
char  *getcwd(char *buf, int n);
int    chdir(const char *p);
void   sleep(int ms);
void   printf(const char *fmt, ...);
void   puts(const char *s);
