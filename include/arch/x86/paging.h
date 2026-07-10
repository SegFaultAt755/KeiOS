#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define PAGING_KERNEL_START 0xC0000000
#define PAGING_KERNEL_END   0xFFFFF000

#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define MAX_PHYSICAL_BYTES 0x100000000ULL /* 4GB */

struct [[gnu::packed]] page_table_entry {
    uint32_t present   : 1;
    uint32_t rw        : 1;
    uint32_t user      : 1;
    uint32_t pwt       : 1;
    uint32_t pcd       : 1;
    uint32_t accessed  : 1;
    uint32_t dirty     : 1;
    uint32_t pat       : 1;
    uint32_t global    : 1;
    uint32_t available : 3;
    uint32_t frame     : 20;
};

struct [[gnu::packed]] page_dir_entry {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t ignored    : 1;
    uint32_t page_size  : 1;
    uint32_t ignored2   : 4;
    uint32_t table_addr : 20;
};

/* Aligned tables architecture */
struct [[gnu::aligned(PAGE_SIZE)]] page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRIES];
};

struct [[gnu::aligned(PAGE_SIZE)]] page_dir {
    struct page_dir_entry entries[PAGE_TABLE_ENTRIES];
};

void paging_initialize(uint32_t mem_high_point, uint32_t physical_alloc_start);
