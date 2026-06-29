#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 256

typedef struct [[gnu::packed]] GateDescriptor {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  reserved;
    uint8_t  attributes;
    uint16_t offset_high;
} GateDescriptor;

typedef struct [[gnu::packed]] Idtr {
    uint16_t size;
    uint32_t offset;
} Idtr;

void idt_initialize(void);

#endif