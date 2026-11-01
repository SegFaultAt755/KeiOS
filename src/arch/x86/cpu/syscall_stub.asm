global syscall_stub

syscall_stub:
    cli
    pusha
    push esp

    extern syscall_handler
    call syscall_handler

    add esp, 4
    cmp eax, 1
    je syscall_return_to_kernel
    popa

    iret

syscall_return_to_kernel:
    mov eax, [esp + 28]
    mov esp, [user_saved_kernel_esp]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

extern user_saved_kernel_esp
