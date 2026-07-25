/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/shell/cmd.h"

#include "config.h"
#include "drivers/cmos.h"
#include "drivers/terminal.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/reboot.h"
#include "libkern/bcd.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

extern uint32_t tick;

void cmd_help(void) {
    kprintf("Available commands:\n");
    kprintf("\thelp      Show this help message\n");
    kprintf("\tclear     Clear the screen\n");
    kprintf("\techo      Print text to the screen\n");
    kprintf("\tver       Show OS version\n");
    kprintf("\tuptime    Show system uptime in ticks\n");
    kprintf("\tmeminfo   Show memory usage and kernel footprint\n");
    kprintf("\tcolor     Change terminal text color (1-15 or name)\n");
    kprintf("\tdatetime  Show current date and time\n");
    kprintf("\treboot    Reboot the system\n");
    kprintf("\thalt      Halt the CPU\n");
}

void cmd_clear(void) {
    terminal_clear();
}

void cmd_echo(const char *args) {
    if (args && *args != '\0') {
        kprintf("%s\n", args);
    } else {
        kprintf("\n");
    }
}

void cmd_ver(void) {
    kprintf("KeiOS %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void cmd_uptime(void) {
    int secs = tick / 1000;
    int mins = secs / 60;
    int hours = mins / 60;

    kprintf("%d:%02d:%02d.%03d\n", hours, mins % 60, secs % 60, tick % 1000);
}

void cmd_meminfo(void) {
    extern uint32_t _kernel_start;
    extern uint32_t _kernel_end;

    uint32_t start = (uint32_t)&_kernel_start;
    uint32_t end = (uint32_t)&_kernel_end;
    uint32_t size = end - start;

    kprintf("System Memory Information:\n");
    kprintf("  Kernel Start: 0x%08x\n", start);
    kprintf("  Kernel End:   0x%08x\n", end);
    kprintf("  Kernel Size:  %d KB (%d bytes)\n", size / 1024, size);
}

void cmd_color(const char *args) {
    if (!args || *args == '\0') {
        kprintf("Usage: color <number 1-15> or <color name>\n");
        kprintf("Colors: 1=blue 2=green 3=cyan 4=red 5=magenta 6=brown 7=white\n");
        kprintf("        8=gray 9=lblue 10=lgreen 11=lcyan 12=lred 13=lmagenta 14=lyellow 15=lwhite\n");
        cmd_help();
        return;
    }

    int color = -1;

    if (strcmp(args, "blue") == 0)
        color = 1;
    else if (strcmp(args, "green") == 0)
        color = 2;
    else if (strcmp(args, "cyan") == 0)
        color = 3;
    else if (strcmp(args, "red") == 0)
        color = 4;
    else if (strcmp(args, "magenta") == 0)
        color = 5;
    else if (strcmp(args, "brown") == 0)
        color = 6;
    else if (strcmp(args, "white") == 0)
        color = 7;
    else if (strcmp(args, "gray") == 0)
        color = 8;
    else if (strcmp(args, "lblue") == 0)
        color = 9;
    else if (strcmp(args, "lgreen") == 0)
        color = 10;
    else if (strcmp(args, "lcyan") == 0)
        color = 11;
    else if (strcmp(args, "lred") == 0)
        color = 12;
    else if (strcmp(args, "lmagenta") == 0)
        color = 13;
    else if (strcmp(args, "lyellow") == 0)
        color = 14;
    else if (strcmp(args, "lwhite") == 0)
        color = 15;
    else {
        bool is_num = true;
        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] < '0' || args[i] > '9') {
                is_num = false;
                break;
            }
        }

        if (is_num)
            color = atoi(args);
    }

    if (color == 0) {
        kprintf("Error: Color 0 (black) makes text invisible on a black background.\n");
        cmd_help();
        return;
    }

    if (color < 1 || color > 15) {
        kprintf("Invalid color specified. Please choose between 1 and 15.\n");
        cmd_help();
        return;
    }

    terminal_set_color((uint8_t)color);
}

void cmd_datetime(void) {
    uint8_t sec = bcd_to_binary(read_cmos_reg(CMOS_SEC));
    uint8_t min = bcd_to_binary(read_cmos_reg(CMOS_MIN));
    uint8_t hour = bcd_to_binary(read_cmos_reg(CMOS_HOUR));
    uint8_t day = bcd_to_binary(read_cmos_reg(CMOS_DAY));
    uint8_t month = bcd_to_binary(read_cmos_reg(CMOS_MONTH));
    uint8_t year = bcd_to_binary(read_cmos_reg(CMOS_YEAR));

    kprintf("20%02d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, min, sec);
}

void cmd_reboot(void) {
    kprintf("Rebooting...\n");
    reboot();

    while (true) {
        disable_interrupts();
        halt();
    }
}

void cmd_halt(void) {
    kprintf("Halting CPU...\n");

    while (true) {
        disable_interrupts();
        halt();
    }
}
