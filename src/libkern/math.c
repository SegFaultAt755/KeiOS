/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/math.h"

float fabsf(float n) {
    return (n < 0.0f) ? -n : n;
}

float sqrtf(float n) {
    if (n < 0.0f) {
        union {
            unsigned int i;
            float f;
        } nan = {0x7FC00000};
        return nan.f;
    }

    float res;
    __asm__ volatile("sqrtss %1, %0" : "=x"(res) : "x"(n));
    return res;
}

float sinf(float n) {
    /* Range formula: n = n - (2*PI) * round(n / (2*PI)) */
    float two_pi = 2.0f * PI;
    int quotients = (int)(n / two_pi);
    n = n - (quotients * two_pi);

    if (n > PI)
        n -= two_pi;
    if (n < -PI)
        n += two_pi;

    /* Bring n into the tightest Taylor window: sin(PI - x) = sin(x) */
    if (n > PI / 2.0f)
        n = PI - n;
    else if (n < -PI / 2.0f)
        n = -PI - n;

    /* 7th degree Taylor series */
    float n2 = n * n;
    float term = n;
    float sum = n;

    term = -term * n2 / 6.0f; /* -n^3 / 3! */
    sum += term;

    term = -term * n2 / 20.0f; /* +n^5 / 5! */
    sum += term;

    term = -term * n2 / 42.0f; /* -n^7 / 7! */
    sum += term;

    return sum;
}

float cosf(float n) {
    return sinf((PI / 2.0f) - n);
}
