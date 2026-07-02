#include "kernel/interrupt.h"
#include "libkern/stdio.h"
#include "drivers/terminal.h"
#include "drivers/timer.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "config.h"

volatile uint64_t tick = 0;
void timer_callback(Registers *regs) {
    tick += 1;
}

void show_banner(void);

[[noreturn]]
void kernel_entry(void) {
    /* Initialize terminal */
    terminal_initialize();

    /* Show welcome message */
    kprintf(LOG_EMPTY, "Welcome to %s %d.%d.%d! ", NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, TERMINAL_DEFAULT_BACKGROUND_COLOR));
        kprintf(LOG_EMPTY, "<3\n");
        terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR));

    show_banner();

    /* Initialization */
    gdt_initialize();
    idt_initialize();
    timer_initialize(100, timer_callback); /* Every 65,535 Hz passed - calling callback */
    enable_interrupts();

    /* Infinite loop to prevent CPU fault */
    while (true) {
        __asm__ volatile ("cli; hlt"); /* Disable interrupts and halt */
    }
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