/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"
#include "drivers/cmos.h"
#include "drivers/pit.h"
#include "drivers/terminal.h"
#include "drivers/vga.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"

#if (ARCH == X86)
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/paging.h"
#elif (ARCH == X64)

#endif

#include "libkern/bcd.h"
#include "libkern/stdio.h"

volatile uint64_t tick = 0;
void pit_callback(struct registers *) {
    tick += 1;
}

void show_banner(void);
void memory_initialize(struct multiboot_info *mbi);

[[noreturn]] void kernel_entry(uint32_t, struct multiboot_info *mbi) {
    /* Initialize kernel */
    qemu_printf(QEMU_INFO, "Loading kernel");
    gdt_initialize();
    idt_initialize();
    pit_initialize(100, pit_callback);
    memory_initialize(mbi);
    enable_interrupts();

    /* Initialize graphics */
    vga_init_text();
    terminal_initialize((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);

    /* Show welcome message */
    qemu_printf(QEMU_INFO, "Show greeting messages");
    kprintf("Welcome to %s %d.%d.%d! ", NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    terminal_set_color(vga_entry_color(VGA_8B_LIGHT_RED, TERMINAL_DEFAULT_BG));
    kprintf("<3\n");
    terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG));
    show_banner();

    /* Infinite loop to prevent CPU fault */
    while (true) {
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

void memory_initialize(struct multiboot_info *mbi) {
    qemu_printf(QEMU_INFO, "Initializing memory");

    uint32_t mod_addr = *(uint32_t *)(mbi->mods_addr + 4);
    uint32_t physical_alloc_start = (mod_addr + 0xFFF) & ~0xFFF;
    qemu_printf(QEMU_INFO, "Physical allocation start point is from 0x%x", physical_alloc_start);

    uint64_t mem_high_point = mbi->mem_upper * 1024;
    if (mem_high_point > MAX_PHYSICAL_BYTES)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_INFO, "Memory high point is from 0x%x", mem_high_point);

    paging_initialize((uint32_t)mem_high_point, physical_alloc_start);
}
