#pragma once

#include <stdint.h>

typedef struct [[gnu::packed]] PageTableEntry {
    uint32_t present   : 1; /* 1 = The page is in physical memory */
    uint32_t rw        : 1; /* 0 = Read-only 1 = read/write */
    uint32_t user      : 1; /* 0 = Supervisor 1 = user mode */
    uint32_t pwt       : 1; /* Write through caching */
    uint32_t pcd       : 1; /* Cache disable */
    uint32_t accessed  : 1; /* Set by cpu when read/written */
    uint32_t dirty     : 1; /* Set by cpu when written to */
    uint32_t pat       : 1; /* Page attribute table */
    uint32_t global    : 1; /* Prevents tlb flush on cr3 reload */
    uint32_t available : 3; /* Custom bits for kernel */
    uint32_t frame     : 20; /* High 20 bits of the physical frame address */
} PageTableEntry;

typedef struct [[gnu::packed]] PageDirectoryEntry {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t ignored    : 1;
    uint32_t page_size  : 1;  /* 0 = 4kb pages 1 = 4mb large pages */
    uint32_t ignored2   : 4;
    uint32_t table_addr : 20; /* High 20 bits of the page table physical address */
} PageDirectoryEntry;

/* Aligned tables architecture */
typedef struct [[gnu::aligned(4096)]] PageTable {
    PageTableEntry entries[1024];
} PageTable;

typedef struct [[gnu::aligned(4096)]] PageDirectory {
    PageDirectoryEntry entries[1024];
} PageDirectory;

void paging_load_directory(PageDirectory *dir);
void paging_enable(void);
void paging_flush_tlb(uint32_t addr);
