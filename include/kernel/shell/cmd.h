#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

struct builtin_cmd {
    const char *name;
    void (*handler)(void);
    void (*handler_args)(const char *);
};

void cmd_help(void);
void cmd_clear(void);
void cmd_echo(const char *args);
void cmd_ver(void);
void cmd_uptime(void);
void cmd_meminfo(void);
void cmd_reboot(void);
void cmd_halt(void);
void cmd_color(const char *args);
void cmd_datetime(void);
