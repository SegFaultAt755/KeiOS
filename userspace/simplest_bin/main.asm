; Using 'ret' instruction for better debugging. Normally it has to be 'jmp $' or something like that
; But I need to check if it really gets into the user program and changes eax to DEADBEEF

section .text
global _start
_start:
    mov eax, 0xDEADBEEF
    ret
