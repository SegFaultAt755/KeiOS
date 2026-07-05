#pragma once

#include <stdint.h>

[[noreturn]] void runtime_panic(const char *reason, const char *file, uint32_t line);

#define KERNEL_PANIC(reason) runtime_panic(reason, __FILE__, __LINE__)
#define KERNEL_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            runtime_panic(message, __FILE__, __LINE__); \
        } \
    } while (0)
