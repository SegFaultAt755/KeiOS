/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0;

    .text ALIGN(4):
    {
        *entry.o(.text)
        *(.text)
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .rodata ALIGN(4):
    {
        *(.rodata*)
    }
}
