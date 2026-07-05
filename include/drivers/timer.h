#pragma once

#include <stdint.h>
#include "arch/x86/isr.h"

void timer_initialize(uint32_t frequency, void (*callback)(Registers *regs));
