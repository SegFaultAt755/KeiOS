#pragma once

#include <stdint.h>
#include "libkern/stdio.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define CMOS_SECOND 0x00
#define CMOS_MINUTE 0x02
#define CMOS_HOUR   0x04
#define CMOS_WEEK   0x06
#define CMOS_DAY    0x07
#define CMOS_MONTH  0x08
#define CMOS_YEAR   0x09

uint8_t read_cmos_register(uint8_t reg);
