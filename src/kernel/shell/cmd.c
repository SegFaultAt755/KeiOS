/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/shell/cmd.h"

#include "config.h"
#include "drivers/cmos.h"
#include "drivers/pit.h"
#include "drivers/terminal.h"

#include "kernel/halt.h"
#include "kernel/interrupts.h"
#include "kernel/reboot.h"

#include "libkern/bcd.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

void cmd_help() {
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

void cmd_clear() {
    terminal_clear();
}

void cmd_echo(const char *args) {
    if (args && *args != '\0') {
        kprintf("%s\n", args);
    } else {
        kprintf("\n");
    }
}

void cmd_ver() {
    kprintf("KeiOS %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void cmd_uptime() {
    auto secs = pit_ticks / 1000;
    auto mins = secs / 60;
    auto hours = mins / 60;

    kprintf("%02ld:%02d:%02d.%03d\n", hours, (uint8_t)(mins % 60), (uint8_t)(secs % 60), (uint16_t)(pit_ticks % 1000));
}

void cmd_meminfo() {
    extern uint32_t _kernel_start;
    extern uint32_t _kernel_end;

    auto start = (uint32_t)&_kernel_start;
    auto end = (uint32_t)&_kernel_end;
    auto size = end - start;

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

    auto color = -1;

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
        auto is_num = true;
        for (auto i = 0; args[i] != '\0'; i++) {
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

void cmd_datetime() {
    auto sec = bcd_to_binary(read_cmos_reg(CMOS_SEC));
    auto min = bcd_to_binary(read_cmos_reg(CMOS_MIN));
    auto hour = bcd_to_binary(read_cmos_reg(CMOS_HOUR));
    auto day = bcd_to_binary(read_cmos_reg(CMOS_DAY));
    auto month = bcd_to_binary(read_cmos_reg(CMOS_MONTH));
    auto year = bcd_to_binary(read_cmos_reg(CMOS_YEAR));

    kprintf("20%02d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, min, sec);
}

void cmd_reboot() {
    kprintf("Rebooting...\n");
    reboot();

    while (true) {
        disable_interrupts();
        halt();
    }
}

void cmd_halt() {
    kprintf("Halting CPU...\n");

    while (true) {
        disable_interrupts();
        halt();
    }
}
