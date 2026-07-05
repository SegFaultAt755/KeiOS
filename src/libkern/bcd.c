#include "libkern/bcd.h"

uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}
