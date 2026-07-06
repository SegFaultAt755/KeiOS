#include "arch/x86/isr.h"
#include "kernel/panic.h"
#include "libkern/stdio.h"

Isr interrupt_handlers[256];
void register_interrupt_handler(uint8_t number, Isr handler) {
    interrupt_handlers[number] = handler;
}

void divide_by_zero_fault_handler(Registers *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    KERNEL_PANIC_FORMAT("Division by zero", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_address, regs -> eip
    );
}

void invalid_opcode_fault_handler(Registers *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    KERNEL_PANIC_FORMAT("Invalid opcode", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_address, regs -> eip
    );
}

void stack_segment_fault_handler(Registers *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    KERNEL_PANIC_FORMAT("Stack segment", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_address, regs -> eip
    );
}

void general_protection_fault_handler(Registers *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    KERNEL_PANIC_FORMAT("General protection", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_address, regs -> eip
    );
}

void double_fault_handler(Registers*) {
    KERNEL_PANIC("Double fault", 0);
}

void page_fault_handler(Registers *regs) {
    uint32_t faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    int present  = !(regs -> error_code & 0x1); /* Page not present */
    int write    = regs -> error_code & 0x2;    /* Write operation failed */
    int user     = regs -> error_code & 0x4;    /* Fault occurred in user mode */
    int reserved = regs -> error_code & 0x8;    /* Overwrote protected CPU bits */
    int id       = regs -> error_code & 0x10;   /* Caused by an instruction fetch */

    KERNEL_PANIC_FORMAT("Page fault",
        "Received error codes:\n\t\tPresent: %d\n\t\tWrite: %d\n\t\tUser: %d\n\t\tReserved: %d\n\t\tId: %d",
        present, write, user, reserved, id
    );
}

void isr_handler(Registers regs) {
    switch (regs.interrupt_number) {
        case 0:
            divide_by_zero_fault_handler(&regs);
            break;
        case 6:
            invalid_opcode_fault_handler(&regs);
            break;
        case 8:
            double_fault_handler(&regs);
            break;
        case 12:
            stack_segment_fault_handler(&regs);
            break;
        case 13:
            general_protection_fault_handler(&regs);
            break;
        case 14:
            page_fault_handler(&regs);
            break;
        default:
            KERNEL_PANIC("Unhandled interrupt", 0);
    }
}
