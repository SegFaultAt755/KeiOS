/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"

#include "drivers/cpio.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"

#include "kernel/graphics.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "kernel/shell/shell.h"
#include "kernel/userspace/enter.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/features.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "kernel/core/cpio.h"
#include "kernel/core/mem.h"
#include "kernel/core/mods.h"
#include "kernel/core/time.h"

#define MULTIBOOT_MAGIC 0x2BADB002

extern uint32_t _kernel_start;

[[noreturn]] void kernel_entry(uint32_t magic, struct multiboot_info *mbi) {
    if (magic != MULTIBOOT_MAGIC)
        goto halt;

    /* Early initialization */
    gdt_init();
    idt_init();
    cpu_feat_init();

    /* Memory */
    struct multiboot_info *mbi_virt = mbi;
    if ((uintptr_t)mbi < KERNEL_VIRTUAL_OFFSET)
        mbi_virt = (struct multiboot_info *)((uintptr_t)mbi + KERNEL_VIRTUAL_OFFSET);

    mem_init(mbi_virt);

    /* Modules and filesystem */
    const uint32_t mod_count = mb_parse_mods(mbi_virt, mod_cb, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "[MB] Info: addr=%p flags=%u count=%u", mbi_virt, mbi_virt->flags, mod_count);

    cpio_parse(cpio_cb, nullptr);

    /* Hardware and drivers */
    pit_init(1193, pit_cb);
    ps2_init();
    gfx_init(mbi_virt);

    /* Handoff */
    enable_interrupts();

    if (exec_init == nullptr || exec_init_size == 0)
        KERNEL_PANIC("No initial executable found or valid",
                     "Initial executable pointer is null or the executable size equals 0");

    qemu_printf(QEMU_KERN, QEMU_INFO, "Handing off initial executable: addr=%p size=%u bytes", exec_init,
                exec_init_size);
    execute_init_binary(exec_init, exec_init_size);

    /* Fallback */
    goto halt;
halt:
    while (true) {
        disable_interrupts();
        halt();
    }
}
