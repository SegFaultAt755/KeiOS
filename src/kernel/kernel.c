/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"

#include "drivers/cpio.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/sleep.h"

#include "kernel/graphics.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
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
#include "libkern/string.h"

#include <stddef.h>
#include <stdint.h>

uint8_t *exec_init = nullptr;

static inline void pit_callback(struct registers *) {
    pit_ticks += 1;
}

static inline void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    pit_ticks += ms;
}

void cpio_callback_function(
    const char *name, [[maybe_unused]] struct cpio_header header,
    const uint8_t *data, [[maybe_unused]] size_t data_len, [[maybe_unused]] void *user_context
) {
    const char *exec_name = "bin/main.elf";
    if (strcmp(name, exec_name) == 0) {
        exec_init = (uint8_t *)data;
    }
}

static void module_callback(struct multiboot_parsed_module *mod, uint32_t index, void *) {
    qemu_printf(QEMU_KERN, QEMU_INFO, "Module %u: (start=%p, size=%u bytes, cmd='%s')", index, mod->start_addr,
                mod->size, mod->cmdline);

    /* Parse CPIO */
    const char *initramfs_cmd = "initramfs";
    if (strcmp(mod->cmdline, initramfs_cmd) == 0) {
        struct cpio_info info;
        info.base_addr = (const uint8_t *)mod->start_addr;
        info.size = mod->size;

        if (cpio_initialize(info) != 0) { /* Important to work properly */
            qemu_printf(QEMU_DRV, QEMU_PANIC, "Failed to initialize CPIO parser");
            KERNEL_PANIC("CPIO Parser", "Failed to initialize CPIO parser");
        }
    }
}

[[noreturn]] void kernel_entry(uint32_t magic, struct multiboot_info *mbi) {
    if (magic != 0x2BADB002) {
        disable_interrupts();
        halt();
    }

    /* Initialize kernel */
    gdt_initialize();
    idt_initialize();
    pit_initialize(1193, pit_callback);

    /* Parse multiboot */
    auto multiboot_mods_count = multiboot_parse_modules(mbi, module_callback, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "Multiboot info: (address: %p, flags: %d, count: %d)", mbi, mbi->flags,
                multiboot_mods_count);

    memory_initialize(mbi);
    initialize_cpu_features();

    /* Parse CPIO */
    cpio_parse(cpio_callback_function, nullptr);

    /* Kernel level drivers */
    ps2_initialize();

    /* Enabling interrupts */
    enable_interrupts();

    /* Load first user program */
    if (exec_init == nullptr)
        KERNEL_PANIC("No initial executable", "Initial executable data pointer is null, maybe failed to parse it from CPIO archive");

#if 0
    /* Initialize graphics */
    auto graphics = graphics_initialize(mbi);

    /* Initialize user space */
    if (graphics == GRAPHICS_TYPE_TEXT_MODE) {
        shell_initialize();
    }
#endif

    /* Infinite loop to prevent CPU fault */
    goto halt;
halt:
    while (true) {
        enable_interrupts();
        halt();
    }
}
