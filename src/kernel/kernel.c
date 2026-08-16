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

#include "kernel/core/initramfs.h"
#include "kernel/core/multiboot_modules.h"
#include "kernel/core/time_handler.h"
#include "kernel/core/memory_init.h"

extern uint32_t _kernel_start;

[[noreturn]] void kernel_entry(uint32_t magic, struct multiboot_info *mbi) {
    if (magic != 0x2BADB002)
        goto halt;

    /* Early initialization */
    gdt_initialize();
    idt_initialize();
    initialize_cpu_features();

    /* Memory */
    struct multiboot_info *mbi_virtual = mbi;
    if ((uintptr_t)mbi < KERNEL_VIRTUAL_OFFSET)
        mbi_virtual = (struct multiboot_info *)((uintptr_t)mbi + KERNEL_VIRTUAL_OFFSET);

    memory_initialize(mbi_virtual);

    /* Hardware and drivers */
    pit_initialize(1193, pit_callback);
    ps2_initialize();

    /* Modules and filesystem */
    auto multiboot_mods_count = multiboot_parse_modules(mbi_virtual, module_callback, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "Multiboot info: (address: %p, flags: %d, count: %d)", mbi_virtual, mbi_virtual->flags,
                multiboot_mods_count);
    
    cpio_parse(cpio_callback_function, nullptr);

    /* Handoff */
    enable_interrupts();

    if (exec_init == nullptr || exec_init_size == 0)
        KERNEL_PANIC("No initial executable",
                     "Initial executable data pointer is null");

    qemu_printf(QEMU_KERN, QEMU_INFO, "Handoff initial executable: (address: %p, size: %d bytes)", exec_init, exec_init_size);
    execute_init_binary(exec_init, exec_init_size);

    /* Fallback */
    goto halt;
halt:
    while (true) {
        disable_interrupts();
        halt();
    }
}
