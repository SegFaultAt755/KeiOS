#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

inline void halt() {
    __asm__ volatile ("hlt");
}
