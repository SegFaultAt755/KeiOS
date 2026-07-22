/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "shell/shell.h"

#include "config.h"
#include "drivers/cmos.h"
#include "drivers/ps2.h"
#include "drivers/terminal.h"
#include "kernel/halt.h"
#include "kernel/qemu.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

extern uint32_t tick;

#define INPUT_BUF_SIZE 256

static char input_buffer[INPUT_BUF_SIZE];
static int input_len = 0;

/* Forward declarations for built-in commands */
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(const char *args);
static void cmd_ver(void);
static void cmd_uptime(void);
static void cmd_meminfo(void);
static void cmd_reboot(void);
static void cmd_halt(void);
static void cmd_color(const char *args);
static void cmd_datetime(void);

/* Command table entry */
typedef struct {
    const char *name;
    void (*handler)(void);
    void (*handler_args)(const char *);
} builtin_cmd_t;

static const builtin_cmd_t builtins[] = {
    {"help", cmd_help, NULL},
    {"clear", cmd_clear, NULL},
    {"echo", NULL, cmd_echo},
    {"ver", cmd_ver, NULL},
    {"uptime", cmd_uptime, NULL},
    {"meminfo", cmd_meminfo, NULL},
    {"reboot", cmd_reboot, NULL},
    {"halt", cmd_halt, NULL},
    {"color", NULL, cmd_color},
    {"datetime", cmd_datetime, NULL},
    {NULL, NULL, NULL},
};

static void shell_print_prompt(void) {
    kprintf("keios> ");
}

/* ---- Built-in command implementations ---- */

static void cmd_help(void) {
    kprintf("Available commands:\n");
    kprintf("  help      Show this help message\n");
    kprintf("  clear     Clear the screen\n");
    kprintf("  echo      Print text to the screen\n");
    kprintf("  ver       Show OS version\n");
    kprintf("  uptime    Show system uptime in ticks\n");
    kprintf("  meminfo   Show memory usage\n");
    kprintf("  color     Change terminal color\n");
    kprintf("  datetime  Show current date and time\n");
    kprintf("  reboot    Reboot the system\n");
    kprintf("  halt      Halt the CPU\n");
}

static void cmd_clear(void) {
    terminal_clear();
    shell_print_prompt();
}

static void cmd_echo(const char *args) {
    if (args)
        kprintf("%s\n", args);
    else
        kprintf("\n");
}

static void cmd_ver(void) {
    kprintf("KeiOS %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

static void cmd_uptime(void) {
    kprintf("Ticks: %d\n", tick);
}

static void cmd_meminfo(void) {
    /* Heap stats — report what we can from the linker symbol */
    extern uint32_t _kernel_end;
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    kprintf("Kernel end: 0x%x\n", kernel_end);
    kprintf("Heap starts after kernel end (4MB configured)\n");
}

static void cmd_color(const char *args) {
    if (!args) {
        kprintf("Usage: color <number>\n");
        kprintf("Colors: 0=black 1=blue 2=green 3=cyan 4=red 5=magenta 6=brown 7=white\n");
        kprintf("        8=gray 9=lblue 10=lgreen 11=lcyan 12=lred 13=lmagenta 14=lyellow 15=lwhite\n");
        return;
    }

    int color = atoi(args);
    if (color < 0 || color > 15) {
        kprintf("Invalid color number (0-15)\n");
        return;
    }

    terminal_set_color((uint8_t)color);
}

static void cmd_datetime(void) {
    uint8_t sec = read_cmos_reg(CMOS_SEC);
    uint8_t min = read_cmos_reg(CMOS_MIN);
    uint8_t hour = read_cmos_reg(CMOS_HOUR);
    uint8_t day = read_cmos_reg(CMOS_DAY);
    uint8_t month = read_cmos_reg(CMOS_MONTH);
    uint8_t year = read_cmos_reg(CMOS_YEAR);

    kprintf("20%d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, min, sec);
}

static void cmd_reboot(void) {
    kprintf("Rebooting...\n");
    /* Triple fault to reboot */
    __asm__ volatile("lidt (%%eax)" : : "a"(0));
    __asm__ volatile("int $3");
    while (1) {
        halt();
    }
}

static void cmd_halt(void) {
    kprintf("Halting CPU.\n");
    while (1) {
        halt();
    }
}

/* ---- Shell core ---- */

static void shell_execute(const char *line) {
    /* Skip leading whitespace */
    while (*line == ' ')
        line++;

    if (*line == '\0')
        return;

    /* Copy line for tokenizing (strtok modifies the string) */
    char buf[INPUT_BUF_SIZE];
    strcpy(buf, line);

    /* Extract command name */
    char *cmd = strtok(buf, " ");
    if (!cmd)
        return;

    /* Extract remaining arguments (everything after the command) */
    const char *args = NULL;
    const char *rest = line + strlen(cmd);
    while (*rest == ' ')
        rest++;
    if (*rest != '\0')
        args = rest;

    /* Look up command in builtin table */
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(cmd, builtins[i].name) == 0) {
            if (builtins[i].handler_args && args)
                builtins[i].handler_args(args);
            else if (builtins[i].handler)
                builtins[i].handler();
            else
                kprintf("Unknown command: %s\n", cmd);
            return;
        }
    }

    kprintf("Unknown command: %s\n", cmd);
}

void shell_key_handler(uint16_t key) {
    if (key == '\n') {
        terminal_putchar('\n');
        input_buffer[input_len] = '\0';
        if (input_len > 0)
            shell_execute(input_buffer);
        input_len = 0;
        shell_print_prompt();
    } else if (key == '\b') {
        if (input_len > 0) {
            input_len--;
            terminal_putchar('\b');
        }
    } else if (key >= 32 && key <= 126 && input_len < INPUT_BUF_SIZE - 1) {
        input_buffer[input_len++] = (char)key;
        terminal_putchar((char)key);
    }
}

void shell_initialize(void) {
    ps2_set_key_callback(shell_key_handler);
    shell_print_prompt();
}
