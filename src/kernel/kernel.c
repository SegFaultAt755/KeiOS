/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"

#include "drivers/cpio.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"

#include "kernel/acpi.h"
#include "kernel/graphics.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "kernel/rsdp.h"
#include "kernel/shell/shell.h"
#include "kernel/userspace/enter.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/features.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "kernel/core/cpio.h"
#include "kernel/core/mem.h"
#include "kernel/core/mods.h"
#include "kernel/core/time.h"

[[noreturn]] void kernel_entry(uint32_t magic, struct multiboot_info *mbi) {
    if (magic != 0x2BADB002)
        goto halt;

    /* Perform early kernel initialization */
    gdt_init();
    idt_init();
    cpu_feat_init();

    uint32_t *rsdp_addr = find_rsdp_addr();
    if (rsdp_addr == nullptr)
        KERNEL_PANIC("Failed to find RSDP start address",
                     "Incompatible device or BIOS failed to load data onto memory");
    else
        qemu_printf(QEMU_KERN, QEMU_OK, "RSDP start address found at %p", rsdp_addr);

    struct rsdp *rsdp = (struct rsdp *)ADD_KERNEL_OFFSET((uintptr_t)rsdp_addr);

    /* Initialize memory management */
    struct multiboot_info *mbi_virt = mbi;
    if ((uintptr_t)mbi < KERNEL_VIRTUAL_OFFSET)
        mbi_virt = (struct multiboot_info *)ADD_KERNEL_OFFSET((uintptr_t)mbi);

    mem_init(mbi_virt);

    acpi_init(rsdp);

    /* Load kernel modules and initialize the filesystem */
    const uint32_t mod_count = mb_parse_mods(mbi_virt, mod_cb, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "[MB] Info: addr=%p flags=%u count=%u", mbi_virt, mbi_virt->flags, mod_count);

    cpio_parse(cpio_cb, nullptr);

    /* Initialize hardware and device drivers */
    pit_init(1193, pit_cb);
    ps2_init();
    gfx_init(mbi_virt);

    /* Transfer control to the user-space stage */
    enable_interrupts();

    if (exec_init == nullptr || exec_init_size == 0)
        KERNEL_PANIC("No initial executable found or valid",
                     "Initial executable pointer is null or the executable size equals 0");

    qemu_printf(QEMU_KERN, QEMU_INFO, "Handing off initial executable: addr=%p size=%u bytes", exec_init,
                exec_init_size);
    execute_init_binary(exec_init, exec_init_size);

    /* Fallback for unhandled errors */
    goto halt;
halt:
    while (true) {
        disable_interrupts();
        halt();
    }
}
