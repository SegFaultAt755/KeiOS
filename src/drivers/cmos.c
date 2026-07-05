#include "drivers/cmos.h"

uint8_t read_cmos_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}
