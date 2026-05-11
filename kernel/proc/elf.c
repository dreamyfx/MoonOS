// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#include "elf.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../lib/kstring.h"
#include "../fs/vfs.h"

#define ET_EXEC 2
#define PT_LOAD 1
#define EM_X86_64 62

typedef struct __attribute__((packed)) {
    uint8_t  ident[16];
    uint16_t type, machine;
    uint32_t version;
    uint64_t entry, phoff, shoff;
    uint32_t flags, ehsize;
    uint16_t phentsize, phnum, shentsize, shnum, shstrndx;
} elf64_ehdr_t;

typedef struct __attribute__((packed)) {
    uint32_t type, flags;
    uint64_t offset, vaddr, paddr, filesz, memsz, align;
} elf64_phdr_t;

uint64_t elf_load(pagemap_t pm, const char *path) {
    int fd = vfs_open(path, 0);
    if (fd < 0) return 0;

    elf64_ehdr_t ehdr;
    vfs_read(fd, &ehdr, sizeof(ehdr));

    if (ehdr.ident[0] != 0x7F || ehdr.ident[1] != 'E' ||
        ehdr.ident[2] != 'L'  || ehdr.ident[3] != 'F') {
        vfs_close(fd);
        return 0;
    }
    if (ehdr.machine != EM_X86_64) { vfs_close(fd); return 0; }

    for (int i = 0; i < ehdr.phnum; i++) {
        elf64_phdr_t phdr;
        vfs_seek(fd, ehdr.phoff + i * ehdr.phentsize);
        vfs_read(fd, &phdr, sizeof(phdr));
        if (phdr.type != PT_LOAD) continue;

        uint64_t pages = (phdr.memsz + PAGE_SIZE - 1) / PAGE_SIZE;
        uint64_t vbase = phdr.vaddr & ~(uint64_t)(PAGE_SIZE - 1);
        uint64_t vmflags = VMM_PRESENT | VMM_USER;
        if (phdr.flags & 2) vmflags |= VMM_WRITE;

        for (uint64_t p = 0; p < pages; p++) {
            uint64_t phys = pmm_alloc();
            vmm_map(pm, vbase + p * PAGE_SIZE, phys, vmflags);
        }

        extern uint64_t g_hhdm_offset;
        uint64_t phys_base = vmm_translate(pm, vbase);
        uint8_t *dst = (uint8_t *)(phys_base + g_hhdm_offset + (phdr.vaddr - vbase));

        vfs_seek(fd, phdr.offset);
        vfs_read(fd, dst, phdr.filesz);
        if (phdr.memsz > phdr.filesz)
            k_memset(dst + phdr.filesz, 0, phdr.memsz - phdr.filesz);
    }

    vfs_close(fd);
    return ehdr.entry;
}
