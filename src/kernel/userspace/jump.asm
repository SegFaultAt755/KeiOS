; SPDX-License-Identifier: GPLv3
; Copyright (C) 2026 KeiOS Developers

global jump_to_user
extern user_saved_kernel_esp

jump_to_user:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov [user_saved_kernel_esp], esp
    mov eax, [ebp + 8]
    mov bx, [ebp + 12]

    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    mov esp, eax
    iretd
