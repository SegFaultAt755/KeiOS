#pragma once

#include <stdint.h>

struct [[gnu::packed]] tss_entry {
    uint16_t link;
    uint16_t reserved_link;

    uint32_t esp0; /* Kernel stack pointer */
    uint16_t ss0;  /* Kernel stack segment */
    uint16_t reserved_ss0;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_ss1;
    
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_ss2;
    
    uint32_t cr3; /* Page directory base */
    uint32_t eip; /* Instruction pointer */
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t reserved_es;
    uint16_t cs;
    uint16_t reserved_cs;
    uint16_t ss;
    uint16_t reserved_ss;
    uint16_t ds;
    uint16_t reserved_ds;
    uint16_t fs;
    uint16_t reserved_fs;
    uint16_t gs;
    uint16_t reserved_gs;

    uint16_t ldtr;
    uint16_t reserved_ldtr;
    
    uint32_t trap;
    uint32_t iomap;
};

void write_tss(uint16_t ss0, uint32_t esp0);
