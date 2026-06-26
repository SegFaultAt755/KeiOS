#include "kernel/terminal.h"

void kernel_entry(void) {
    /* Initialize terminal */
    terminal_initialize();

    /* Show welcome message */
    terminal_write("Welcome to KeiOS.\n", 18);

    /* Initialize GDT */
    terminal_write("Initializing GDT...\n", 20);
    /* gdt_initialize(); */

    /* infinite loop to prevent CPU fault */
    for (;;) {

    }
}