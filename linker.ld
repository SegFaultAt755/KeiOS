/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

KERNEL_LOAD_ADDR = 0x00100000;
KERNEL_VIRTUAL_OFFSET = 0xC0000000;

ENTRY(_start)
SECTIONS
{
    . = KERNEL_LOAD_ADDR;
    .multiboot ALIGN(4K): {
        *(.multiboot)
    }

    .note.gnu.build-id ALIGN(4K): {
        *(.note.gnu.build-id)
    }

    .boot ALIGN(4K): {
        *(.boot)
    }

    . += KERNEL_VIRTUAL_OFFSET;
    _kernel_start = .;

    .text ALIGN(4K): AT(ADDR(.text) - KERNEL_VIRTUAL_OFFSET) {
        *(.text)
    }

    .data ALIGN(4K): AT(ADDR(.data) - KERNEL_VIRTUAL_OFFSET) {
        *(.data)
    }

    .rodata ALIGN(4K): AT(ADDR(.rodata) - KERNEL_VIRTUAL_OFFSET) {
        *(.rodata)
    }

    .bss ALIGN(4K): AT(ADDR(.bss) - KERNEL_VIRTUAL_OFFSET) {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}