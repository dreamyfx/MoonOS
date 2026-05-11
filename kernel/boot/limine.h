// Copyright (c) 2025-2026 Andrew (dreamyfx)
// MoonOS 1.0.0 x86_64 2026
// This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
// This header should be maintained in any file it is present in, as per the AGPL license terms.
#pragma once
#include <stdint.h>

#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b

#define LIMINE_MEMMAP_USABLE                 0
#define LIMINE_MEMMAP_RESERVED               1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE       2
#define LIMINE_MEMMAP_ACPI_NVS               3
#define LIMINE_MEMMAP_BAD_MEMORY             4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES     6
#define LIMINE_MEMMAP_FRAMEBUFFER            7

struct limine_uuid { uint32_t a; uint16_t b, c; uint8_t d[8]; };

#define LIMINE_BASE_REVISION(n) \
    uint64_t limine_base_revision[3] = { 0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, (n) }

struct limine_framebuffer {
    void    *address;
    uint64_t width, height, pitch;
    uint16_t bpp;
    uint8_t  memory_model, red_mask_size, red_mask_shift;
    uint8_t  green_mask_size, green_mask_shift;
    uint8_t  blue_mask_size, blue_mask_shift;
    uint8_t  unused[7];
    uint64_t edid_size;
    void    *edid;
    uint64_t mode_count;
    void   **modes;
};

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer **framebuffers;
};

struct limine_framebuffer_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_framebuffer_response *response;
};

#define LIMINE_FRAMEBUFFER_REQUEST { LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b }

struct limine_memmap_entry { uint64_t base, length; uint64_t type; };

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    struct limine_memmap_entry **entries;
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_memmap_response *response;
};

#define LIMINE_MEMMAP_REQUEST { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 }

struct limine_hhdm_response { uint64_t revision; uint64_t offset; };
struct limine_hhdm_request  { uint64_t id[4]; uint64_t revision; struct limine_hhdm_response *response; };
#define LIMINE_HHDM_REQUEST { LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b }

struct limine_file {
    uint64_t revision;
    void    *address;
    uint64_t size;
    char    *path, *cmdline;
    uint32_t media_type;
    uint32_t unused;
    uint32_t tftp_ip, tftp_port;
    uint32_t partition_index;
    uint32_t mbr_disk_id;
    struct limine_uuid gpt_disk_uuid, gpt_part_uuid, part_uuid;
};

struct limine_module_response {
    uint64_t revision;
    uint64_t module_count;
    struct limine_file **modules;
};

struct limine_module_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_module_response *response;
    uint64_t internal_module_count;
    void   **internal_modules;
};

#define LIMINE_MODULE_REQUEST { LIMINE_COMMON_MAGIC, 0x3e7e279702be32af, 0xca1c4f3bd1280cee }

struct limine_smp_info;
struct limine_smp_response {
    uint64_t revision;
    uint32_t flags;
    uint32_t bsp_lapic_id;
    uint64_t cpu_count;
    struct limine_smp_info **cpus;
};

struct limine_smp_info {
    uint32_t processor_id, lapic_id;
    uint64_t reserved;
    void (*goto_address)(struct limine_smp_info *);
    uint64_t extra_argument;
};

struct limine_smp_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_smp_response *response;
    uint64_t flags;
};

#define LIMINE_SMP_REQUEST { LIMINE_COMMON_MAGIC, 0x95a67b819a1b857e, 0xa0b61b723b6a73e0 }

struct limine_rsdp_response { uint64_t revision; void *address; };
struct limine_rsdp_request  { uint64_t id[4]; uint64_t revision; struct limine_rsdp_response *response; };
#define LIMINE_RSDP_REQUEST { LIMINE_COMMON_MAGIC, 0xc5e77b6b397e7b43, 0x27637845accdcf3c }

struct limine_kernel_file_response { uint64_t revision; struct limine_file *kernel_file; };
struct limine_kernel_file_request  { uint64_t id[4]; uint64_t revision; struct limine_kernel_file_response *response; };
#define LIMINE_KERNEL_FILE_REQUEST { LIMINE_COMMON_MAGIC, 0xad97e90e83f1ed67, 0x31eb5d1c5ff23b69 }
