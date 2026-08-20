#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/*
    Note:
        sleep and sleep_ms use 100% CPU time, so other work stops until they return.
        Use the PIT directly in most situations.
        sleep_init and sleep_deinitialize are optional. They enable and disable channel 2,
        which drives the PC speaker
*/

void sleep_init(void);
void sleep(uint16_t ticks);
void sleep_ms(uint32_t ms);
void sleep_deinitialize(void);
