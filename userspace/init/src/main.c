/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

typedef unsigned long int uint;

extern uint syscall(uint eax, uint ebx);

int main([[maybe_unused]] int argc, [[maybe_unused]] int *argv) {
    uint result = syscall(0, 0);
    return result;
}
