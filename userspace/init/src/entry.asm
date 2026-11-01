; SPDX-License-Identifier: GPLv3
; Copyright (C) 2026 KeiOS Developers

section .data
    argv dd 1, 2, 77567, 4, 5
    argc equ ($ - argv) / 4

section .text
%define SYSCALL_EXIT 6

global _start
_start:
    push argv
    push argc

    extern main
    call main

    mov ebx, eax
    mov eax, SYSCALL_EXIT
    int 0x80

.hang:
    cli
    hlt
    jmp .hang
