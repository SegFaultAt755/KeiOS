#include "libkern/stdio.h"
#include "kernel/terminal.h"

void kernel_entry(void) {
    /* Initialize terminal */
    terminal_initialize();

    /* Show welcome message */
    kprintf(LOG_EMPTY, "Welcome to KeiOS <3\n");

    /* Initialize GDT */
    /*
    terminal_writestring("Initializing GDT...\n");
    gdt_initialize();
    */

    /* Infinite loop to prevent CPU fault */
    for (;;) {

    }
}