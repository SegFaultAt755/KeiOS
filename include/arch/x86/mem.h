#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "kernel/multiboot.h"

#define PAGE_SIZE          4096
#define PAGE_TABLE_ENTRIES 1024
#define MAX_PHYSICAL_BYTES 0x100000000ULL /* 4GB */

#define KERNEL_START 0xC0000000

/* Paging flags */
#define PTE_PRESENT  (1U << 0)   /* Present, is loaded in memory */
#define PTE_RW       (1U << 1)   /* Read/write, allowed to write to page if set */
#define PTE_USER     (1U << 2)   /* User/supervisor, user-mode code can access page if set */
#define PTE_PWT      (1U << 3)   /* Page-level write-through, enables write-through caching */
#define PTE_PCD      (1U << 4)   /* Page-level cache disable, disables caching for the page */
#define PTE_ACCESSED (1U << 5)   /* Accessed, set by hardware when page is read or written */
#define PTE_DIRTY    (1U << 6)   /* Dirty, set by hardware when page is written to */
#define PTE_PAT      (1U << 7)   /* Page attribute table, selects memory attributes with pwt/pcd */
#define PTE_GLOBAL   (1U << 8)   /* Global, prevents tlb flush on cr3 reload */
#define PTE_FRAME    0xFFFFF000U /* Page frame address, mask for physical 4KB page address */

#define PDE_PRESENT  (1U << 0)   /* Present, is loaded in memory */
#define PDE_RW       (1U << 1)   /* Read/write, allowed to write to mapped range if set */
#define PDE_USER     (1U << 2)   /* User/supervisor, user-mode code can access mapped range if set */
#define PDE_PWT      (1U << 3)   /* Page-level write-through, enables write-through for page table */
#define PDE_PCD      (1U << 4)   /* Page-level cache disable, disables caching for page table */
#define PDE_ACCESSED (1U << 5)   /* Accessed, set by hardware when directory range is accessed */
#define PDE_PS       (1U << 7)   /* Page size, maps a direct 4MB page if set */
#define PDE_FRAME    0xFFFFF000U /* Page frame address, mask for physical page table address */

typedef uint32_t page_table_t[PAGE_TABLE_ENTRIES] [[gnu::aligned(PAGE_SIZE)]];
typedef uint32_t page_dir_t[PAGE_TABLE_ENTRIES] [[gnu::aligned(PAGE_SIZE)]];

/* Initialization */
void memory_initialize(struct multiboot_info *mbi);
