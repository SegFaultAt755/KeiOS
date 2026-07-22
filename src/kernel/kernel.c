/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"
#include "drivers/display.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/sleep.h"
#include "drivers/terminal.h"
#include "kernel/graphics.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"
#include "kernel/shell.h"

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

uint32_t tick = 0;
static inline void pit_callback(struct registers *) {
    tick += 1;
}

static inline void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    tick += ms;
}

void module_callback(struct multiboot_parsed_module *mod, uint32_t index, void *) {
    qemu_printf(QEMU_KERN, QEMU_INFO, "Module %u: (start=%p, size=%u bytes, cmd='%s')", index, mod->start_addr,
                mod->size, mod->cmdline);
}

void show_banner(void);
void memory_initialize(struct multiboot_info *mbi);

[[noreturn]] void kernel_entry(uint32_t, struct multiboot_info *mbi) {
    qemu_set_time_var(&tick);

    /* Initialize kernel */
    tick_wait(1); /* Manually freeze the execution for better debugging experience */
    gdt_initialize();
    tick_wait(1);
    idt_initialize();
    tick_wait(1);
    pit_initialize(1193, pit_callback);

    /* Parse multiboot */
    uint32_t multiboot_mods_count = multiboot_parse_modules(mbi, module_callback, NULL);
    qemu_printf(QEMU_KERN, QEMU_INFO, "Multiboot info: (address: 0x%x, flags: %d, count: %d)", mbi, mbi->flags,
                multiboot_mods_count);

    memory_initialize(mbi);
    initialize_cpu_features();

    /* Kernel level drivers */
    ps2_initialize();

    /* Enabling interrupts */
    enable_interrupts();

    int graphics = get_graphics_type(mbi);
    if (graphics == GRAPHICS_TYPE_TEXT_MODE) {
        /* Initialize VGA text mode */
        vga_init_text();
        terminal_initialize((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);

        /* Show welcome message */
        kprintf("Welcome to KeiOS %d.%d.%d! ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        terminal_set_color(vga_entry_color(VGA_8B_LIGHT_RED, TERMINAL_DEFAULT_BG));
        kprintf("<3\n");
        terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG));
        show_banner();

        shell_initialize();
    } else if (graphics == GRAPHICS_TYPE_FRAMEBUFFER) {
        struct display_info info;
        info.flags = mbi->flags;
        info.width = mbi->framebuffer_width;
        info.height = mbi->framebuffer_height;
        info.pitch = mbi->framebuffer_pitch;
        info.bpp = mbi->framebuffer_bpp;

        uint32_t phys_addr = (uint32_t)mbi->framebuffer_addr;
        uint32_t virt_addr = 0xE0000000;
        uint32_t fbo_size = info.pitch * info.height;

        qemu_printf(QEMU_DRV, QEMU_INFO, "Framebuffer address info: (physical: 0x%x, virtual: 0x%x, FBO size: %d)",
                    phys_addr, virt_addr, fbo_size);

        bool map_success = true;

        /* Loop through the entire size of the framebuffer and map it page by page */
        for (uint32_t offset = 0; offset < fbo_size; offset += PAGE_SIZE) {
            if (!vmm_map_page(virt_addr + offset, phys_addr + offset, PTE_PRESENT | PTE_RW | PTE_PWT)) {
                map_success = false;
                break;
            }
        }

        if (!map_success) {
            qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to map linear framebuffer to virtual memory");
            info.lfb_addr = nullptr;
        }

        info.lfb_addr = (uint32_t *)virt_addr;

        display_initialize(info);
        display_clear(0x00141414);
    }

    /* Infinite loop to prevent CPU fault */
    goto halt;
halt:
    while (true) {
        enable_interrupts();
        halt();
    }
}

void show_banner(void) {
    const char *banner[] = {" _  __        _   ___    ____", "| |/ /  ___  (_) / _ \\  / ___|",
                            "| ' /  / _ \\ | || | | | \\__ \\", "| . \\ |  __/ | || |_| | ___) |",
                            "|_|\\_\\ \\___| |_| \\___/ |____/"};

    enum vga_8b_colors rainbow[] = {VGA_8B_LIGHT_RED,   VGA_8B_LIGHT_BROWN, /* VGA equivalent to
                                                                               orange/yellow */
                                    VGA_8B_LIGHT_GREEN, VGA_8B_LIGHT_CYAN,  VGA_8B_LIGHT_BLUE, VGA_8B_LIGHT_MAGENTA};

    for (int row = 0; row < 5; row++) {
        for (int col = 0; banner[row][col] != '\0'; col++) {
            /* Calculate the diagonal rainbow color */
            int color_idx = (row + col) % 5;
            uint8_t color = vga_entry_color(rainbow[color_idx], TERMINAL_DEFAULT_BG);

            terminal_set_color(color);
            char ch_str[2] = {banner[row][col], '\0'};
            kprintf(ch_str);
        }

        terminal_blankline();
    }

    uint8_t default_color = vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG);
    terminal_set_color(default_color);

    for (uint8_t i = 0; i < 5; i++)
        terminal_blankline();
}
