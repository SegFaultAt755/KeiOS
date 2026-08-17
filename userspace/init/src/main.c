/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "sys/syscall.h"
#include <stdint.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] int *argv) {
    uint32_t result = syscall(0, 0, 0, 0);
    return result;
}
