#include "kernel/qemu.h"
#include "libkern/string.h"
#include <stddef.h>

static void qemu_print(const char *str) {
    for (size_t i = 0; i < strlen(str); i++)
        qemu_write_char(str[i]);
}

void qemu_printf(QemuLogLevel level, const char *fmt, ...) {
    switch (level) {
        case QEMU_LOG_INFO:    qemu_print("[QEMU INFO]: "); break;
        case QEMU_LOG_WARNING: qemu_print("[QEMU WARN]: "); break;
        case QEMU_LOG_ERROR:   qemu_print("[QEMU ERROR]: "); break;
        default: break;
    }

    va_list args;
    va_start(args, fmt);

    char buffer[1024] = { 0 };
    ksnprintf(buffer, 1024, fmt, args);
    qemu_print(buffer);

    va_end(args);
}
