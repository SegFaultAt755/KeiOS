#include "arch/x86/isr.h"
#include "kernel/panic.h"
#include "libkern/stdio.h"

static inline uint32_t get_fault_addr(void) {
    uint32_t addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(addr));
    return addr;
}

isr_t isr_intr_handler[256];
void intr_handler(uint8_t num, isr_t hld) {
    isr_intr_handler[num] = hld;
}

void divide_by_zero_fault_handler(struct registers *regs) {
    KERNEL_PANIC_FORMAT("Division by zero fault", "CPU denied to divide a number by zero / data: %x | code: %x",
        get_fault_addr(), regs->eip
    );
}

void invalid_opcode_fault_handler(struct registers *regs) {
    KERNEL_PANIC_FORMAT("Invalid opcode fault", "CPU cannot decode code or a pointer leads to invalid data / data: %x | code: %x",
        get_fault_addr(), regs->eip
    );
}

void stack_segment_fault_handler(struct registers *regs) {
    KERNEL_PANIC_FORMAT("Stack segment fault", "Stack corrupted or stack overflow occured / data: %x | code: %x",
        get_fault_addr(), regs->eip
    );
}

void general_protection_fault_handler(struct registers *regs) {
    KERNEL_PANIC_FORMAT("General protection fault", "Not enough permissions to execute code / data: %x | code: %x",
        get_fault_addr(), regs->eip
    );
}

void double_fault_handler(struct registers *) {
    KERNEL_PANIC("Double fault", "Failed to handle a fault");
}

void page_fault_handler(struct registers *regs) {
    int present  = !(regs->err & 0x1);
    int write    = regs->err & 0x2;
    int user     = regs->err & 0x4;
    int reserved = regs->err & 0x8;
    int id       = regs->err & 0x10;

    KERNEL_PANIC_FORMAT("Page fault",
        "Memory corrupted or failed to access to memory / data: %x | code: %x / %d|%d|%d|%d|%d",
        get_fault_addr(), regs->eip, present, write, user, reserved, id
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
            KERNEL_PANIC_FORMAT("Unhandled interrupt", "Interrupt number: %d", regs.intr_num);
    }
}
