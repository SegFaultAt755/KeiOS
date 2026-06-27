#ifndef __STDIO_H__
#define __STDIO_H__

/*
Note: Initialize terminal first to show clear output with kprintf
*/

#include <stdarg.h>

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

#endif