#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>

typedef struct [[gnu::packed]] GdtEntry {
    uint16_t limit_low;   /* The lower 16 bits of the limit */
    uint16_t base_low;    /* The lower 16 bits of the base */
    uint8_t  base_middle; /* The next 8 bits of the base */
    uint8_t  access;      /* Access flags */
    uint8_t  granularity; /* High 4 bits of limit + 4 bits of flags */
    uint8_t  base_high;   /* The last 8 bits of the base */
} GdtEntry;

typedef struct [[gnu::packed]] GdtPointer {
    uint16_t limit; /* Size of the GDT array minus 1 */
    uint32_t base;  /* Linear address of the GDT array */
} GdtPointer;

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_initialize(void);

#endif