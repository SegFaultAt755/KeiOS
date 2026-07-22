#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stddef.h>

size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
char *strtok(char *str, const char *delim);
char *strdup(const char *s);
int atoi(const char *s);
void itoa(int val, char *buf, int base);
int uvalue_to_str(char *buf, unsigned int val, int base, int min_w, int zpad);
