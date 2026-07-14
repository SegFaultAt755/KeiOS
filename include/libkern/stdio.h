#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

void kprint_uint(unsigned int val, int base);
void kprint_int(int val);
void kvprintf(const char *fmt, va_list args);
void kprintf(const char *fmt, ...);
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int ksnprintf(char *buf, size_t size, const char *fmt, ...);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void waitb(uint32_t n) {
    for (uint32_t i = 0; i < n; i++)
        outb(0x80, 0);
} 
