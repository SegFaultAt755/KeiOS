#include "arch/x86/isr.h"
#include "kernel/panic.h"
#include "libkern/stdio.h"

isr isr_intr_handler[256];
void intr_handler(uint8_t num, isr hld) {
    isr_intr_handler[num] = hld;
}

void divide_by_zero_fault_handler(struct registers *regs) {
    uint32_t faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));

    KERNEL_PANIC_FORMAT("Division by zero", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_addr, regs -> eip
    );
}

void invalid_opcode_fault_handler(struct registers *regs) {
    uint32_t faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));

    KERNEL_PANIC_FORMAT("Invalid opcode", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_addr, regs -> eip
    );
}

void stack_segment_fault_handler(struct registers *regs) {
    uint32_t faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));

    KERNEL_PANIC_FORMAT("Stack segment", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_addr, regs -> eip
    );
}

void general_protection_fault_handler(struct registers *regs) {
    uint32_t faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));

    KERNEL_PANIC_FORMAT("General protection", "From code:\n\t\tData address: %x\n\t\tCode address: %x",
        faulting_addr, regs -> eip
    );
}

void double_fault_handler(struct registers *) {
    KERNEL_PANIC("Double fault", 0);
}

void page_fault_handler(struct registers *regs) {
    uint32_t faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));

    int present  = !(regs -> err & 0x1); /* Page not present */
    int write    = regs -> err & 0x2;    /* Write operation failed */
    int user     = regs -> err & 0x4;    /* Fault occurred in user mode */
    int reserved = regs -> err & 0x8;    /* Overwrote protected CPU bits */
    int id       = regs -> err & 0x10;   /* Caused by an instruction fetch */

    KERNEL_PANIC_FORMAT("Page fault",
        "Received error codes:\n\t\tPresent: %d\n\t\tWrite: %d\n\t\tUser: %d\n\t\tReserved: %d\n\t\tId: %d",
        present, write, user, reserved, id
    );
}

void isr_handler(struct registers regs) {
    switch (regs.intr_num) {
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
