/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "config.h"
#include "drivers/pit.h"
#include "drivers/sleep.h"
#include "drivers/terminal.h"
#include "drivers/vbe.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "libkern/stdio.h"

uint32_t tick = 0;
void pit_callback(struct registers *) {
    tick += 1;
}

static inline void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    tick += ms;
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

    memory_initialize(mbi);
    enable_interrupts();

    /* Logging */
    qemu_printf(QEMU_KERN, QEMU_INFO, "Multiboot info: (address: 0x%x)", mbi);

    /* Initialize graphics */
    vbe_initialize(mbi);
    vbe_set_pixel(0, 0, 0x00FF0000);

#if 0
    vga_init_text();
    terminal_initialize((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);

    /* Show welcome message */
    kprintf("Welcome to %s %d.%d.%d! ", "KeiOS", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    terminal_set_color(vga_entry_color(VGA_8B_LIGHT_RED, TERMINAL_DEFAULT_BG));
    kprintf("<3\n");
    terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG));
    show_banner();
#endif

    /* Infinite loop to prevent CPU fault */
    goto halt;
halt:
    while (true) {
        disable_interrupts();
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
