; SPDX-License-Identifier: GPLv3
; Copyright (C) 2026 KeiOS Developers

global jump_to_user

jump_to_user:
    mov eax, [esp + 4]
    mov bx,  [esp + 8]

    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    mov esp, eax
    iretd
