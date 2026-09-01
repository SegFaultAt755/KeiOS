/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/rsdp.h"
#include "kernel/shell/shell.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "kernel/core/mem.h"

/* Forward declarations for boot initialization functions */
extern void boot_init_early_cpu(void);
extern struct rsdp *boot_init_rsdp(void);
extern struct multiboot_info *boot_init_get_mbi_virtual(struct multiboot_info *mbi);
extern void boot_init_acpi_system(struct rsdp *rsdp);
extern void boot_init_load_modules(struct multiboot_info *mbi_virt);
extern void boot_init_cpio_filesystem(void);
extern void boot_init_hardware_drivers(struct multiboot_info *mbi_virt);

[[noreturn]] void kernel_entry(uint32_t magic, struct multiboot_info *mbi) {
    if (magic != 0x2BADB002)
        goto halt;

    /* Early CPU subsystem setup */
    boot_init_early_cpu();

    /* RSDP and ACPI initialization */
    struct rsdp *rsdp = boot_init_rsdp();

    /* Memory management initialization */
    struct multiboot_info *mbi_virt = boot_init_get_mbi_virtual(mbi);
    mem_init(mbi_virt);

    /* ACPI subsystem setup */
    boot_init_acpi_system(rsdp);

    /* Filesystem initialization */
    boot_init_load_modules(mbi_virt);
    boot_init_cpio_filesystem();

    /* Hardware driver initialization */
    boot_init_hardware_drivers(mbi_virt);

    /* Enable interrupts and start the shell */
    enable_interrupts();
    shell_init();

    /* Fallback for unhandled errors */
halt:
    while (true) {
        halt();
    }
}
