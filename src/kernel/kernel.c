#include "terminal.h"

void kernel_entry(void) {
    /* Initialize terminal */
    terminal_initialize();
    terminal_write("Hello, World!", 13);

    /* infinite loop to prevent CPU fault */
    for (;;) {

    }
}