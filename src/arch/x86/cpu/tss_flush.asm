; SPDX-License-Identifier: GPLv3
; Copyright (C) 2026 KeiOS Developers

global tss_flush

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret
