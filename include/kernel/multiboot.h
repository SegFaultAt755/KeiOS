#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include <stddef.h>

#define MULTIBOOT_INFO_MEMORY           (1 << 0)
#define MULTIBOOT_INFO_BOOTDEV          (1 << 1)
#define MULTIBOOT_INFO_CMDLINE          (1 << 2)
#define MULTIBOOT_INFO_MODS             (1 << 3)
#define MULTIBOOT_INFO_AOUT_SYMS        (1 << 4)
#define MULTIBOOT_INFO_ELF_SHDR         (1 << 5)
#define MULTIBOOT_INFO_MEM_MAP          (1 << 6)
#define MULTIBOOT_INFO_DRIVE_INFO       (1 << 7)
#define MULTIBOOT_INFO_CONFIG_TABLE     (1 << 8)
#define MULTIBOOT_INFO_BOOT_LOADER_NAME (1 << 9)
#define MULTIBOOT_INFO_APM_TABLE        (1 << 10)
#define MULTIBOOT_INFO_VBE_INFO         (1 << 11)
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO (1 << 12)

#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

struct [[gnu::packed]] multiboot_aout_symbol_table {
    uint32_t tab_size;
    uint32_t str_size;
    uint32_t addr;
    uint32_t reserved;
};

struct [[gnu::packed]] multiboot_elf_section_header_table {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
};

struct [[gnu::packed]] multiboot_mmap_entry {
    uint32_t size;
    uint32_t addr_lower;
    uint32_t addr_upper;
    uint32_t len_lower;
    uint32_t len_upper;
    uint32_t type;
};

struct [[gnu::packed]] multiboot_module {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t reserved;
};

struct [[gnu::packed]] multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;

    uint32_t cmd_line;
    uint32_t mods_count;
    uint32_t mods_addr;

    union {
        struct multiboot_aout_symbol_table aout_symbol;
        struct multiboot_elf_section_header_table elf_section;
    } un;

    uint32_t mmap_len;
    uint32_t mmap_addr;

    uint32_t drives_len;
    uint32_t drives_addr;

    uint32_t config_table;
    uint32_t boot_loader_name;

    uint32_t apm_table;

    /* VBE fields */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_offset;
    uint16_t vbe_interface_len;

    /* Direct linear framebuffer fields */
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    
    /* Color layout info */
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };

        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
};

struct multiboot_parsed_module {
    const void *start_addr;
    const void *end_addr;
    size_t size;
    const char *cmdline;
};

typedef void (*multiboot_module_callback_t)(struct multiboot_parsed_module *mod, uint32_t index, void *data);

bool multiboot_has_modules(struct multiboot_info *mbi);
uint32_t multiboot_parse_modules(struct multiboot_info *mbi, multiboot_module_callback_t callback, void *data);
