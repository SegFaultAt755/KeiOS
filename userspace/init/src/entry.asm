; SPDX-License-Identifier: GPLv3
; Copyright (C) 2026 KeiOS Developers

section .data
    argv dd 1, 2, 77567, 4, 5
    argc equ ($ - argv) / 4

section .text
    push argv
    push argc

    extern main
    call main

    jmp $
