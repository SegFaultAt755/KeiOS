; Declare constants
MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
MBUSEGFX equ  (1 << 0) | (1 << 1)
MBFLAGS  equ  MBALIGN | MEMINFO | MBUSEGFX
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + MBFLAGS)

; Declare multiboot header
section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM
    dd 0, 0, 0, 0, 0

    ; Graphics
    dd 0    ; Graphic mode
    dd 1024 ; Width
    dd 768  ; Height
    dd 32   ; Depth

; Align the stack
section .bss
    align 16
stack_bottom:
    resb 16384 * 8
stack_top:

section .boot
global _start
_start:
    mov ecx, (initial_page_directory - 0xC0000000)
    mov cr3, ecx
    
    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    jmp higher

section .text
higher:
    mov esp, stack_top
    push ebx
    push eax
    xor ebp, ebp

    extern kernel_entry
    call kernel_entry

halt:
    hlt
    jmp halt

section .data
    align 4096
global initial_page_directory
initial_page_directory:
    dd 0x83
    times 768-1 dd 0

    dd (0 << 22) | 0x83
    dd (1 << 22) | 0x83
    dd (2 << 22) | 0x83
    dd (3 << 22) | 0x83
    times 256-4 dd 0
