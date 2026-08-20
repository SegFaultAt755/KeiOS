#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "kernel/multiboot.h"

#define PAGE_SIZE          4096U
#define PAGE_TABLE_ENTRIES 1024U
#define PAGE_MASK          (PAGE_SIZE - 1U)
#define MAX_PHYSICAL_BYTES 0x1000'0000'0ULL /* Maximum physical memory: 4 GB */

#define KERNEL_LOAD_ADDR      0x0010'0000U
#define KERNEL_VIRTUAL_OFFSET 0xC000'0000U
#define ADD_KERNEL_OFFSET(value) ((value) + KERNEL_VIRTUAL_OFFSET)

#define KERNEL_PAGE_ALIGN(value) (((value) + PAGE_MASK) & ~PAGE_MASK)

/* Flags used by page-table entries. */
#define PTE_PRESENT  (1U << 0)   /* The page is present in memory */
#define PTE_RW       (1U << 1)   /* The page can be written when set */
#define PTE_USER     (1U << 2)   /* User-mode code can access the page when set */
#define PTE_PWT      (1U << 3)   /* Use write-through caching when set */
#define PTE_PCD      (1U << 4)   /* Disable caching for the page when set */
#define PTE_ACCESSED (1U << 5)   /* Hardware sets this after the page is accessed */
#define PTE_DIRTY    (1U << 6)   /* Hardware sets this after the page is written */
#define PTE_PAT      (1U << 7)   /* Select page attributes using PWT and PCD */
#define PTE_GLOBAL   (1U << 8)   /* Keep this page in the TLB across CR3 reloads */
#define PTE_FRAME    0xFFFF'F000U /* Mask for the physical 4 KB page address */

#define PDE_PRESENT  (1U << 0)   /* The page table is present in memory */
#define PDE_RW       (1U << 1)   /* The mapped range can be written when set */
#define PDE_USER     (1U << 2)   /* User-mode code can access the range when set */
#define PDE_PWT      (1U << 3)   /* Use write-through caching for the page table */
#define PDE_PCD      (1U << 4)   /* Disable caching for the page table when set */
#define PDE_ACCESSED (1U << 5)   /* Hardware sets this after the directory is accessed */
#define PDE_PS       (1U << 7)   /* Map a direct 4 MB page when set */
#define PDE_FRAME    0xFFFF'F000U /* Mask for the physical page-table address */

typedef uint32_t page_table_t[PAGE_TABLE_ENTRIES] [[gnu::aligned(PAGE_SIZE)]];
typedef uint32_t page_dir_t[PAGE_TABLE_ENTRIES] [[gnu::aligned(PAGE_SIZE)]];
