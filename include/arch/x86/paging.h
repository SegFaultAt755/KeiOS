#pragma once

#include <stdint.h>

#define PAGING_KERNEL_START 0xC0000000
#define PAGING_KERNEL_END   0xFFFFF000

#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define MAX_PHYSICAL_BYTES 0x100000000ULL /* 4GB */

typedef struct [[gnu::packed]] PageTableEntry {
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
} PageTableEntry;

typedef struct [[gnu::packed]] PageDirectoryEntry {
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
} PageDirectoryEntry;

/* Aligned tables architecture */
typedef struct [[gnu::aligned(PAGE_SIZE)]] PageTable {
    PageTableEntry entries[PAGE_TABLE_ENTRIES];
} PageTable;

typedef struct [[gnu::aligned(PAGE_SIZE)]] PageDirectory {
    PageDirectoryEntry entries[PAGE_TABLE_ENTRIES];
} PageDirectory;

void paging_initialize(uint32_t memory_high_point, uint32_t physical_allocation_start);
