/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/shutdown.h"
#include "kernel/acpi.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/panic.h"

[[noreturn]] void shutdown() {
    /* Shut down available components gracefully */
    /* No component shutdown is needed yet */

    shutdown_raw();
}

[[noreturn]] void shutdown_raw() {
    if (!global_fadt)
        KERNEL_PANIC("FADT pointer equals null", "Shutdown happens before FADT was found or FADT is invalid structure");

    acpi_shutdown(global_fadt, global_dsdt);

    while (true) {
        disable_interrupts();
        halt();
    }
}
