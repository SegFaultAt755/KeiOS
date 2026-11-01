/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/userspace/enter.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/heap.h"
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"
#include <stdint.h>

extern uint32_t jump_to_user(struct user_program_regs *, uint32_t);

uint32_t user_saved_kernel_esp;

constexpr uint32_t USER_PROGRAM_BASE = 0x0040'0000U;
constexpr uint32_t USER_STACK_TOP = KERNEL_VIRTUAL_OFFSET;
constexpr uint32_t USER_STACK_PAGE = USER_STACK_TOP - PAGE_SIZE;

void execute_init_binary(const void *cpio_binary, uint32_t binary_size) {
    KERNEL_ASSERT(cpio_binary != nullptr && binary_size > 0, "Invalid init program",
                  "The CPIO init program must have a non-empty binary image");
    KERNEL_ASSERT(binary_size <= UINT32_MAX - (PAGE_SIZE - 1), "Init program is too large",
                  "The CPIO init program size overflows page-count calculation");
    KERNEL_ASSERT(binary_size <= USER_STACK_PAGE - USER_PROGRAM_BASE, "Init program is too large",
                  "The CPIO init program overlaps the user stack");

    uint32_t num_pages = (binary_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t vaddr = USER_PROGRAM_BASE + i * PAGE_SIZE;
        uint32_t phys = pmm_alloc_frame();
        KERNEL_ASSERT(vmm_map_page(vaddr, phys, PTE_RW | PTE_USER), "Init program mapping failed",
                      "Could not map an init program page into user memory");
    }

    memcpy((void *)USER_PROGRAM_BASE, cpio_binary, binary_size);

    uint32_t stack_phys = pmm_alloc_frame();
    KERNEL_ASSERT(vmm_map_page(USER_STACK_PAGE, stack_phys, PTE_RW | PTE_USER), "Init stack mapping failed",
                  "Could not map the init program's user stack");

    struct user_program_regs regs;
    regs.eip = USER_PROGRAM_BASE;
    regs.cs = USER_CS;
    regs.eflags = 0x202;
    regs.esp = USER_STACK_TOP - 4;
    regs.ss = USER_DS;

    qemu_printf(QEMU_KERN, QEMU_INFO, "[USER] Starting CPIO init program (%u bytes)", binary_size);
    uint32_t exit_status = jump_to_user(&regs, USER_DS);
    qemu_printf(QEMU_KERN, QEMU_INFO, "[USER] Init program exited with status %u", exit_status);
}
