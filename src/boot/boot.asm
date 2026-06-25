; declare constants
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; multiboot field
MAGIC    equ  0x1BADB002        ; 'magic number'
CHECKSUM equ -(MAGIC + MBFLAGS)

; declare multiboot header
section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM

; Align the stack
section .bss
    align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    mov esp, stack_top

    extern kernel_entry
    call kernel_entry

    cli
.h: hlt
	jmp .h
.end: