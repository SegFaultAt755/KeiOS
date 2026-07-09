#pragma once

#include <stdint.h>

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
};

struct [[gnu::packed]] gdt_pointer {
    uint16_t limit;
    uint32_t base;
};

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_initialize(void);
