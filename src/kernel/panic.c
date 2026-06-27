#include <stdint.h>
#include "kernel/terminal.h"

#define PANIC_PREFIX "Kei is very sad: "

#if UINTPTR_MAX == UINT32_MAX
    uintptr_t __stack_chk_guard = 0xe2dee396;
#else
    uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
#endif

void __attribute__((noreturn)) __stack_chk_fail(void) {
    terminal_initialize();
    terminal_writestring(PANIC_PREFIX "Kernel stack smashed!\n");
    terminal_writestring("Reboot or shutdown the PC!\n");

    while (true) {
        /* Halt CPU */
    }
}

void __attribute__((noreturn)) __stack_chk_fail_local(void) {
    __stack_chk_fail();
}