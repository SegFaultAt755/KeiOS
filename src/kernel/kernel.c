#include "kernel/panic.h"
#include "kernel/interrupts.h"
#include "kernel/halt.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"

#include "drivers/terminal.h"
#include "drivers/timer.h"
#include "drivers/cmos.h"
#include "drivers/vga.h"

#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/paging.h"

#include "arch/x86/paging.h"

#include "libkern/stdio.h"
#include "libkern/bcd.h"
#include "config.h"

uint64_t tick = 0;
void timer_callback(Registers*) {
    /* Note: Don't try to output any messages from here, because the output will be messy */
    tick += 1;
}

void show_banner(void);
void memory_initialize(MultibootInfo *boot_info);

[[noreturn]] void kernel_entry(uint32_t magic, MultibootInfo *boot_info);
[[noreturn]] void kernel_entry(uint32_t, MultibootInfo *boot_info) {
    /* Initialize kernel */
    qemu_printf(QEMU_LOG_INFO, "Initializing kernel");
    gdt_initialize();
    idt_initialize();
    timer_initialize(100, timer_callback); /* Passing frequency and callback function */
    memory_initialize(boot_info);
    enable_interrupts();

#if 0 /* Remove this code temporarily */
    /* Initialize graphics */
    qemu_printf(QEMU_LOG_INFO, "Initializing graphics");
    if (vga_is_graphics_supported()) {
        vga_init_graphics(boot_info);
        qemu_printf(QEMU_LOG_INFO, "Video mode is selected");
    } else {
        qemu_printf(QEMU_LOG_INFO, "Text mode is selected");
        vga_init_text_mode();
        terminal_initialize((uint16_t*) VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    }

    /* Show welcome message */
    qemu_printf(QEMU_LOG_INFO, "Show greeting messages");
    kprintf(LOG_EMPTY, "Welcome to %s %d.%d.%d! ", NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, TERMINAL_DEFAULT_BACKGROUND_COLOR));
        kprintf(LOG_EMPTY, "<3\n");
        terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR));
    show_banner();
#endif

    /* Infinite loop to prevent CPU fault */
    while (true) {}
}

void show_banner(void) {
    const char *banner[] = {
        " _  __        _   ___    ____",
        "| |/ /  ___  (_) / _ \\  / ___|",
        "| ' /  / _ \\ | || | | | \\__ \\",
        "| . \\ |  __/ | || |_| | ___) |",
        "|_|\\_\\ \\___| |_| \\___/ |____/"
    };

    VgaColors rainbow[] = {
        VGA_COLOR_LIGHT_RED,
        VGA_COLOR_LIGHT_BROWN, /* VGA equivalent to orange/yellow */
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_MAGENTA
    };

    for (int row = 0; row < 5; row++) {
        for (int col = 0; banner[row][col] != '\0'; col++) {
            /* Calculate the diagonal rainbow color */
            int color_idx = (row + col) % 5;
            uint8_t color = vga_entry_color(rainbow[color_idx], TERMINAL_DEFAULT_BACKGROUND_COLOR);

            terminal_set_color(color);
            char ch_str[2] = { banner[row][col], '\0' };
            kprintf(LOG_EMPTY, ch_str);
        }

        terminal_blankline();
    }

    uint8_t default_color = vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR);
    terminal_set_color(default_color);

    for (uint8_t i = 0; i < 5; i++)
        terminal_blankline();
}

void memory_initialize(MultibootInfo *boot_info) {
    qemu_printf(QEMU_LOG_INFO, "Initializing memory");

    uint32_t mod_address = *(uint32_t*)(boot_info -> mods_address + 4);
    uint32_t physical_allocation_start = (mod_address + 0xFFF) & ~0xFFF;
    qemu_printf(QEMU_LOG_INFO, "Physical allocation start point is from %x", physical_allocation_start);

    uint64_t memory_high_point = boot_info -> memory_upper * 1024;
    if (memory_high_point > MAX_PHYSICAL_BYTES)
        memory_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_LOG_INFO, "Memory high point is from %x", memory_high_point);

    paging_initialize((uint32_t) memory_high_point, physical_allocation_start);
}
