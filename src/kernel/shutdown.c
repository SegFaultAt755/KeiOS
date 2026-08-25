/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/shutdown.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"

[[noreturn]] void shutdown() {
    /* Shut down available components gracefully */
    /* No component shutdown is needed yet */

    shutdown_raw();
}

[[noreturn]] void shutdown_raw() {

    while (true) {
        disable_interrupts();
        halt();
    }
}
