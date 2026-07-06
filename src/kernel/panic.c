#include "kernel/panic.h"
#include "kernel/interrupts.h"
#include "kernel/halt.h"
#include "kernel/qemu.h"
#include "drivers/terminal.h"
#include "drivers/vga.h"

#define CH(i) (__TIME__[i] ? (uintptr_t) __TIME__[i] : 0x7FU)
#define CD(i) (__DATE__[i] ? (uintptr_t) __DATE__[i] : 0x31U)

#define COMPILE_TIME_SEED \
    (((CH(0) * 0x1F001511U + CH(1) * 0x0A3F1012U + CH(3) * 0x00A1F023U)  + \
      (CH(4) * 0x0002F101U + CH(6) * 0x00003F12U + CH(7) * 0x000001FAU)) ^ \
      (CD(0) << 24 | CD(2) << 16 | CD(4) << 8 | CD(5)))

uintptr_t __stack_chk_guard = (uintptr_t) COMPILE_TIME_SEED;

[[noreturn]] void runtime_panic(const char *reason, const char *desc, const char *file, uint32_t line) {
    disable_interrupts();
    qemu_printf(QEMU_LOG_PANIC, "(%s:%d) %s | %s\n", file, line, reason, desc);

    vga_init_text_mode();
    terminal_initialize((uint16_t*) VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    terminal_set_color(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    terminal_clear();

    terminal_writestring("==========\n");
    terminal_writestring("            KEI IS VERY SAD\n");
    terminal_writestring("==========\n\n");

    terminal_writestring("Reason: "); terminal_writestring(reason);
    terminal_blankline();

    if (*desc != '\0') {
        terminal_writestring("Description: "); terminal_writestring(desc);
        terminal_blankline();
    }
    
    terminal_writestring("File: "); terminal_writestring(file);
    terminal_blankline();
    
    terminal_writestring("Line: ");
    if (line == 0) {
        terminal_writestring("0");
    } else {
        char buffer[12];
        int i = 10;
        buffer[11] = '\0';
        while (line > 0 && i >= 0) {
            buffer[i--] = (line % 10) + '0';
            line /= 10;
        }
        terminal_writestring(&buffer[i + 1]);
    }

    while (true) {halt();}
}

[[noreturn]] void runtime_panic_format(const char *reason, const char *desc, const char *file, uint32_t line, ...) {
    char formatted_desc[1024]; 
    
    va_list args;
    va_start(args, line);
    
    ksnprintf(formatted_desc, sizeof(formatted_desc), desc, args);
    va_end(args);

    disable_interrupts();

    qemu_printf(QEMU_LOG_PANIC, "(%s:%d) %s | %s\n", file, line, reason, formatted_desc);

    vga_init_text_mode();
    terminal_initialize((uint16_t*) VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    terminal_set_color(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    terminal_clear();

    terminal_writestring("==========\n");
    terminal_writestring("            KEI IS VERY SAD\n");
    terminal_writestring("==========\n\n");

    terminal_writestring("Reason: "); terminal_writestring(reason);
    terminal_blankline();

    if (formatted_desc[0] != '\0') {
        terminal_writestring("Description: "); terminal_writestring(formatted_desc);
        terminal_blankline();
    }
    
    terminal_writestring("File: "); terminal_writestring(file);
    terminal_blankline();
    
    terminal_writestring("Line: ");
    if (line == 0) {
        terminal_writestring("0");
    } else {
        char buffer[12];
        int i = 10;
        buffer[11] = '\0';
        while (line > 0 && i >= 0) {
            buffer[i--] = (line % 10) + '0';
            line /= 10;
        }
        terminal_writestring(&buffer[i + 1]);
    }

    while (true) {halt();}
}

[[noreturn]] void __stack_chk_fail(void) {
    runtime_panic("Kernel stack smashed! (buffer overflow detected)", 0, __FILE__, __LINE__);
}

[[noreturn]] void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}
