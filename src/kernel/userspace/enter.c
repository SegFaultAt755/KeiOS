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

#include "libkern/memory.h"
#include <stdint.h>

extern void jump_to_user(struct user_program_regs *, uint32_t);

void execute_init_binary(void *cpio_binary, uint32_t binary_size) {
    /* Map memory for user space */
    uint32_t num_pages = (binary_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t vaddr = i * PAGE_SIZE; /* Start at virtual address 0 */
        uint32_t phys = pmm_alloc_frame();
        vmm_map_page(vaddr, phys, PTE_PRESENT | PTE_RW | PTE_USER);
    }

    /* Copy the init program to virtual address 0 */
    memcpy((void *)0, cpio_binary, binary_size);

    /* Allocate and map the user-space stack */
    uint32_t stack_phys = pmm_alloc_frame();
    uint32_t stack_bottom_vaddr = KERNEL_VIRTUAL_OFFSET - PAGE_SIZE;
    vmm_map_page(stack_bottom_vaddr, stack_phys, PTE_PRESENT | PTE_RW | PTE_USER);

    /* Set the registers and jump to user space */
    struct user_program_regs regs;
    regs.eip = 0x0;
    regs.cs = USER_CS;
    regs.eflags = 0x202; /* Enable hardware interrupts */
    regs.esp = KERNEL_VIRTUAL_OFFSET - 4;
    regs.ss = USER_DS;

    jump_to_user(&regs, USER_DS);
}
