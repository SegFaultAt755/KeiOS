#pragma once

#include <stdint.h>

#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

typedef struct MultibootAoutSymbolTable {
    uint32_t tab_size;
    uint32_t str_size;
    uint32_t address;
    uint32_t reserved;
} MultibootAoutSymbolTable;

typedef struct MultibootElfSectionHeaderTable {
    uint32_t number;
    uint32_t size;
    uint32_t address;
    uint32_t shndx;
} MultibootElfSectionHeaderTable;

typedef struct [[gnu::packed]] MultibootMmapEntry {
    uint32_t size;
    uint32_t address_lower;
    uint32_t address_upper;
    uint32_t length_lower;
    uint32_t length_upper;
    uint32_t type;
} MultibootMmapEntry;

typedef struct MultibootInfo {
    uint32_t flags;
    uint32_t memory_lower;
    uint32_t memory_upper;
    uint32_t boot_device;

    uint32_t cmd_line;
    uint32_t mods_count;
    uint32_t mods_address;

    union {
        MultibootAoutSymbolTable aout_symbol;
        MultibootElfSectionHeaderTable elf_section;
    } un;

    uint32_t mmap_length;
    uint32_t mmap_address;

    uint32_t drives_length;
    uint32_t drives_address;

    uint32_t config_table;
    uint32_t boot_loader_name;

    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_segment;
    uint16_t vbe_interface_offset;
    uint16_t vbe_interface_length;
} MultibootInfo;
