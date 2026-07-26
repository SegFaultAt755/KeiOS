/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"

#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/sleep.h"

#include "kernel/graphics.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"
#include "kernel/shell/shell.h"

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

#include "libkern/stdio.h"

#include <stddef.h>
#include <stdint.h>

static inline void pit_callback(struct registers *) {
    pit_ticks += 1;
}

static inline void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    pit_ticks += ms;
}

static void module_callback(struct multiboot_parsed_module *mod, uint32_t index, void *) {
    qemu_printf(QEMU_KERN, QEMU_INFO, "Module %u: (start=%p, size=%u bytes, cmd='%s')", index, mod->start_addr,
                mod->size, mod->cmdline);
}

[[noreturn]] void kernel_entry(uint32_t, struct multiboot_info *mbi) {
    /* Initialize kernel */
    tick_wait(1); /* Manually freeze the execution for better debugging experience */
    gdt_initialize();
    tick_wait(1);
    idt_initialize();
    tick_wait(1);
    pit_initialize(1193, pit_callback);

    /* Parse multiboot */
    auto multiboot_mods_count = multiboot_parse_modules(mbi, module_callback, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "Multiboot info: (address: 0x%x, flags: %d, count: %d)", mbi, mbi->flags,
                multiboot_mods_count);

    memory_initialize(mbi);
    initialize_cpu_features();

    /* Kernel level drivers */
    ps2_initialize();

    /* Enabling interrupts */
    enable_interrupts();

    /* Initialize graphics */
    auto graphics = graphics_initialize(mbi);

    /* Initialize user space */
    if (graphics == GRAPHICS_TYPE_TEXT_MODE) {
        shell_initialize();
    }

    /* Infinite loop to prevent CPU fault */
    goto halt;
halt:
    while (true) {
        enable_interrupts();
        halt();
    }
}
