#pragma once

#include <stdint.h>
#include <stdarg.h>

[[noreturn]] void runtime_panic(const char *reason, const char *desc, const char *file, uint32_t line);
[[noreturn]] void runtime_panic_format(const char *reason, const char *desc, const char *file, uint32_t line, ...);

#define KERNEL_PANIC(reason, desc) runtime_panic(reason, desc, __FILE__, __LINE__)
#define KERNEL_ASSERT(condition, reason, desc) \
    do { \
        if (!(condition)) { \
            runtime_panic(reason, desc, __FILE__, __LINE__); \
        } \
    } while (false)

#define KERNEL_PANIC_FORMAT(reason, desc, ...) \
    runtime_panic_format(reason, desc, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define KERNEL_ASSERT_FORMAT(condition, reason, desc, ...) \
    do { \
        if (!(condition)) { \
            runtime_panic_format(reason, desc, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (false)
