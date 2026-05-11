// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>

typedef struct { volatile int locked; } spinlock_t;
#define SPINLOCK_INIT {0}

static inline uint64_t spinlock_acquire_irqsave(spinlock_t *l) {
    uint64_t flags;
    asm volatile("pushfq; pop %0; cli" : "=r"(flags));
    while (__sync_lock_test_and_set(&l->locked, 1)) {
        while (l->locked) asm volatile("pause");
    }
    return flags;
}
static inline void spinlock_release_irqrestore(spinlock_t *l, uint64_t flags) {
    __sync_lock_release(&l->locked);
    asm volatile("push %0; popfq" : : "r"(flags));
}
static inline void spinlock_acquire(spinlock_t *l) {
    while (__sync_lock_test_and_set(&l->locked, 1)) {
        while (l->locked) asm volatile("pause");
    }
}
static inline void spinlock_release(spinlock_t *l) {
    __sync_lock_release(&l->locked);
}
