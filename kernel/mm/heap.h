// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stddef.h>

void  heap_init(void);
void *kmalloc(size_t size);
void *kcalloc(size_t n, size_t size);
void  kfree(void *ptr);
void *krealloc(void *ptr, size_t size);
