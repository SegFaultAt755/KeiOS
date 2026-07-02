#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

inline void enable_interrupts() {
    __asm__ volatile ("sti");
}

inline void halt() {
    __asm__ volatile ("hlt");
}

#endif