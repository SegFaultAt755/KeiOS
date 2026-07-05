#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

inline void enable_interrupts() {
    __asm__ volatile ("sti");
}

inline void disable_interrupts() {
    __asm__ volatile ("cli");
}

#endif