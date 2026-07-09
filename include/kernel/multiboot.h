#pragma once

#include <stdint.h>

#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

struct [[gnu::packed]] multiboot_aout_symbol_table {
    uint32_t tab_size;
    uint32_t str_size;
    uint32_t address;
    uint32_t reserved;
};

struct [[gnu::packed]] multiboot_elf_section_header_table {
    uint32_t number;
    uint32_t size;
    uint32_t address;
    uint32_t shndx;
};

struct [[gnu::packed]] multiboot_mmap_entry {
    uint32_t size;
    uint32_t address_lower;
    uint32_t address_upper;
    uint32_t length_lower;
    uint32_t length_upper;
    uint32_t type;
};

struct [[gnu::packed]] multiboot_info {
    uint32_t flags;
    uint32_t memory_lower;
    uint32_t memory_upper;
    uint32_t boot_device;

    uint32_t cmd_line;
    uint32_t mods_count;
    uint32_t mods_address;

    union {
        struct multiboot_aout_symbol_table aout_symbol;
        struct multiboot_elf_section_header_table elf_section;
    } un;

    uint32_t mmap_length;
    uint32_t mmap_address;

    uint32_t drives_length;
    uint32_t drives_address;

    uint32_t config_table;
    uint32_t boot_loader_name;

    uint32_t apm_table;

    /* VBE fields */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_segment;
    uint16_t vbe_interface_offset;
    uint16_t vbe_interface_length;

    /* Direct linear framebuffer fields*/
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
