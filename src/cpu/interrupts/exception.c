#include <stdint.h>
#include "libkern/stdio.h"

typedef struct Registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t interrupt_number, error_code; /* Pushed by our macros */
    uint32_t eip, cs, eflags, useresp, ss; /* Pushed automatically by CPU */
} Registers;

void exception_handler(Registers *regs) {
    kprintf(LOG_INFO, "Received exception: %d\n", regs -> interrupt_number);
}