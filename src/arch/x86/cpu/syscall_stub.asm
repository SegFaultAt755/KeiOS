global syscall_stub

syscall_stub:
    pusha 
    push esp 

    extern syscall_handler
    call syscall_handler

    add esp, 4
    popa

    iret
