#include "kernel/qemu.h"
#include "libkern/string.h"
#include "libkern/bcd.h"
#include "drivers/cmos.h"
#include "config.h"
#include <stddef.h>

static void qemu_print(const char *str) {
    for (size_t i = 0; i < strlen(str); i++)
        qemu_write_char(str[i]);
}

static void qemu_print_time(void) {
    qemu_printf(QEMU_LOG_EMPTY, "[%d.%d:%d:%d]: ",
        bcd_to_binary(read_cmos_register(CMOS_DAY)),
        bcd_to_binary(read_cmos_register(CMOS_HOUR)),
        bcd_to_binary(read_cmos_register(CMOS_MINUTE)),
        bcd_to_binary(read_cmos_register(CMOS_SECOND))
    );
}

void qemu_printf(QemuLogLevel level, const char *fmt, ...) {
    switch (level) {
        case QEMU_LOG_INFO:    { qemu_print("[QEMU INFO]: "); qemu_print_time(); } break;
        case QEMU_LOG_WARNING: { qemu_print("[QEMU WARNING]: "); qemu_print_time(); } break;
        case QEMU_LOG_ERROR:   { qemu_print("[QEMU ERROR]: "); qemu_print_time(); } break;
        case QEMU_LOG_PANIC:   { qemu_print("[QEMU PANIC]: "); qemu_print_time(); } break;
        default: break;
    }

    va_list args;
    va_start(args, fmt);

    char buffer[1024] = { 0 };
    ksnprintf(buffer, 1024, fmt, args);
    qemu_print(buffer);

    va_end(args);
}
