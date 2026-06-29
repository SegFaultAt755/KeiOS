global isr_stub_table
extern exception_handler

; Generate a stub without an error code
%macro isr_no_err_stub 1 ; Define a macro name and parameters accepting
global isr_stub_%1       ; Make it global
isr_stub_%1:             ; Declare it with unuqie name using parameter accepted
    push 0               ; Pushing a stub into the stack
    push %1              ; Pushing stub code
    jmp isr              ; Jump into isr handler
%endmacro                ; End macro declaretion

; Generate a stub with an error code
%macro isr_err_stub 1
global isr_stub_%1
isr_stub_%1:
    push %1
    jmp isr
%endmacro

; Generate stubs
isr_no_err_stub 0 ; Division by zero
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6  
isr_no_err_stub 7 ; Invalid opcode
isr_err_stub    8 ; Double fault
isr_no_err_stub 9
isr_err_stub    10 ; Invalid TSS
isr_err_stub    11 ; Segment not present
isr_err_stub    12 ; Stack segment fault
isr_err_stub    13 ; General protection fault
isr_err_stub    14 ; Page fault
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_err_stub    21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

; ISR entry
isr:
    pusha
    push esp
    call exception_handler
    add esp, 4
    popa
    add esp, 8
    iret

; Generate a C array table with stubs
isr_stub_table:
%assign i 0
%rep 32
    dd isr_stub_%+i
%assign i i+1
%endrep