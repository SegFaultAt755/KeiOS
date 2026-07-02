#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include "cpu/isr.h"

void timer_initialize(uint32_t frequency, void (*callback)(Registers *regs)); /* The frequency is Hz */

#endif