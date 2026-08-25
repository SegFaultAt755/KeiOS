#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/stdio.h"
#include "kernel/acpi.h"

extern struct fadt *global_fadt;

void shutdown();
void shutdown_raw();
