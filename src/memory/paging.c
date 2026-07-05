#include "memory/paging.h"

void paging_load_directory(PageDirectory *dir) {
    __asm__ volatile ("mov %0, %%cr3" : : "r" ((uint32_t) dir) : "memory");
}

void paging_enable(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000; /* Set pg bit (bit 31) */
    __asm__ volatile ("mov %0, %%cr0" : : "r" (cr0) : "memory");
}

void paging_flush_tlb(uint32_t addr) {
    __asm__ volatile ("invlpg (%0)" : : "r" (addr) : "memory");
}