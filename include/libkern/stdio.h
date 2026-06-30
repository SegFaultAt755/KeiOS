#ifndef __STDIO_H__
#define __STDIO_H__

/*
    Note: Initialize terminal first to show clear output with kprintf
*/

#include <stdarg.h>
#include <stdint.h>

typedef enum LogLevel {
    LOG_EMPTY,
    LOG_INFO,
    LOG_WARN,
    LOG_ERR,
    LOG_DEBUG
} LogLevel;

void kprint_uint(unsigned int val, int base);
void kprint_int(int val);
void kprintf(LogLevel level, const char *fmt, ...);

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

#endif