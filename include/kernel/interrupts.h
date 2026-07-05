#pragma once

inline void enable_interrupts() {
    __asm__ volatile ("sti");
}

inline void disable_interrupts() {
    __asm__ volatile ("cli");
}
