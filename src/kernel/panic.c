#include <stdint.h>
#include "drivers/terminal.h"
#include "kernel/interrupts.h"
#include "kernel/halt.h"

#define PANIC_PREFIX "Kei is very sad: "

#if UINTPTR_MAX == UINT32_MAX
    uintptr_t __stack_chk_guard = 0xe2dee396;
#else
    uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
#endif

[[noreturn]] void __stack_chk_fail(void) {
    terminal_initialize((uint16_t*) VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    terminal_writestring(PANIC_PREFIX "Kernel stack smashed!\n");
    terminal_writestring("Reboot or shutdown the PC!\n");

    while (true) {
        disable_interrupts();
        halt();
    }
}

[[noreturn]] void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}