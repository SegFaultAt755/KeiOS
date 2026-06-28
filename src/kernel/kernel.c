#include "libkern/stdio.h"
#include "kernel/terminal.h"
#include "cpu/gdt.h"

void show_banner(void);

void kernel_entry(void) {
    /* Initialize terminal */
    terminal_initialize();

    /* Show welcome message */
    kprintf(LOG_EMPTY, "Welcome to KeiOS! ");
        terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, TERMINAL_DEFAULT_BACKGROUND_COLOR));
        kprintf(LOG_EMPTY, "<3\n");
        terminal_set_color(vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR));

    show_banner();

    /* Initialization */
    /* ================================= */
    kprintf(LOG_EMPTY, "Initializing Kernel...\n");
    gdt_initialize();
    /* irq_initialze(); */
    /* pmm_initialize(); */
    /* vmm_initialize(); */
    /* pit_initialize(); */
    /* pcb_initialize(); */
    /* scheduler_initialize(); */
    /* ipc_initialize(); */

    /* Infinite loop to prevent CPU fault */
    for (;;) {

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