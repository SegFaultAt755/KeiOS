#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

enum log_level {
    LOG_EMPTY,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERR,
    LOG_DEBUG
};

void kprint_uint(unsigned int val, int base);
void kprint_int(int val);
void kvprintf(enum log_level level, const char *fmt, va_list args);
void kprintf(enum log_level level, const char *fmt, ...);
int  kvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int  ksnprintf(char *buf, size_t size, const char *fmt, ...);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
