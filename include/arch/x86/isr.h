#pragma once

#include <stdint.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef struct Registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t interrupt_number, error_code; /* Pushed by macros */
    uint32_t eip, cs, eflags, useresp, ss; /* Pushed by CPU */
} Registers;

typedef void (*Isr)(Registers *);
extern Isr interrupt_handlers[256];

void register_interrupt_handler(uint8_t number, Isr handler); 
void isr_handler(Registers regs);
