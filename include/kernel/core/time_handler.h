#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "drivers/sleep.h"
#include "drivers/pit.h"

void pit_callback(struct registers *);
void tick_wait(uint32_t ms);
