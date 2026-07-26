/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/shell/cmd.h"
#include "kernel/shell/shell.h"

#include "drivers/ps2.h"
#include "drivers/terminal.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

static char input_buffer[SHELL_INPUT_BUF_SIZE];
static int input_len = 0;
static int input_pos = 0;

static char history_buffer[SHELL_HISTORY_MAX][SHELL_INPUT_BUF_SIZE];
static int history_count = 0;
static int history_index = -1;

static const struct builtin_cmd builtins[] = {
    {"help", cmd_help, nullptr},         {"clear", cmd_clear, nullptr},   {"echo", nullptr, cmd_echo},
    {"ver", cmd_ver, nullptr},           {"uptime", cmd_uptime, nullptr}, {"meminfo", cmd_meminfo, nullptr},
    {"reboot", cmd_reboot, nullptr},     {"halt", cmd_halt, nullptr},     {"color", nullptr, cmd_color},
    {"datetime", cmd_datetime, nullptr}, {nullptr, nullptr, nullptr},
};

static void shell_print_prompt() {
    kprintf("$ ");
}

static void shell_execute(const char *line) {
    while (*line == ' ')
        line++;

    if (*line == '\0')
        return;

    char buf[SHELL_INPUT_BUF_SIZE];
    strcpy(buf, line);

    auto cmd = strtok(buf, " ");
    if (!cmd)
        return;

    const char *args = nullptr;
    auto rest = line + strlen(cmd);

    while (*rest == ' ')
        rest++;

    if (*rest != '\0')
        args = rest;

    for (auto i = 0; builtins[i].name != nullptr; i++) {
        if (strcmp(cmd, builtins[i].name) == 0) {
            if (builtins[i].handler_args) {
                builtins[i].handler_args(args);
            } else if (builtins[i].handler) {
                if (args != nullptr) {
                    kprintf("Error: Command '%s' does not accept arguments.\n", cmd);
                    cmd_help();
                } else {
                    builtins[i].handler();
                }
            }

            return;
        }
    }

    kprintf("Unknown command: %s\n", cmd);
    cmd_help();
}

static void shell_clear_current_line() {
    while (input_pos < input_len) {
        terminal_cursor_right();
        input_pos++;
    }

    while (input_len > 0) {
        terminal_putchar('\b');
        input_len--;
    }

    input_pos = 0;
    input_buffer[0] = '\0';
}

static void shell_set_current_line(const char *str) {
    shell_clear_current_line();
    strcpy(input_buffer, str);

    input_len = strlen(input_buffer);
    input_pos = input_len;

    kprintf("%s", input_buffer);
}

void shell_key_handler(uint16_t key) {
    if (key == '\n') {
        /* Move visual cursor to the very end of line before printing newline */
        while (input_pos < input_len) {
            terminal_cursor_right();
            input_pos++;
        }

        terminal_putchar('\n');
        input_buffer[input_len] = '\0';

        if (input_len > 0) {
            auto save_idx = history_count % SHELL_HISTORY_MAX;
            strcpy(history_buffer[save_idx], input_buffer);

            history_count++;
            history_index = history_count; /* Reset navigation index to newest */

            shell_execute(input_buffer);
        }

        input_len = 0;
        input_pos = 0;
        shell_print_prompt();

    } else if (key == '\b') {
        if (input_pos > 0) {
            /* Shift buffer left to erase character at input_pos - 1 */
            for (auto i = input_pos - 1; i < input_len; i++)
                input_buffer[i] = input_buffer[i + 1];

            input_pos--;
            input_len--;

            /* Step cursor left once to begin redrawing from deletion point */
            terminal_cursor_left();

            /* Redraw tail of the string */
            for (auto i = input_pos; i < input_len; i++)
                terminal_putchar(input_buffer[i]);

            /* Overwrite old trailing character on screen with a whitespace */
            terminal_putchar(' ');

            /* Return visual cursor back to the deletion point */
            for (auto i = input_pos; i <= input_len; i++)
                terminal_cursor_left();
        }

    } else if (key == KEY_LEFT) {
        if (input_pos > 0) {
            input_pos--;
            terminal_cursor_left();
        }

    } else if (key == KEY_RIGHT) {
        if (input_pos < input_len) {
            input_pos++;
            terminal_cursor_right();
        }

    } else if (key == KEY_UP) {
        auto min_idx = (history_count > SHELL_HISTORY_MAX) ? (history_count - SHELL_HISTORY_MAX) : 0;
        if (history_count > 0 && history_index > min_idx) {
            history_index--;
            shell_set_current_line(history_buffer[history_index % SHELL_HISTORY_MAX]);
        }

    } else if (key == KEY_DOWN) {
        if (history_index < history_count) {
            history_index++;
            if (history_index == history_count) {
                shell_set_current_line("");
            } else {
                shell_set_current_line(history_buffer[history_index % SHELL_HISTORY_MAX]);
            }
        }

    } else if (key >= 32 && key <= 126 && input_len < SHELL_INPUT_BUF_SIZE - 1) {
        /* Shift buffer right from input_pos to make room for insertion */
        for (auto i = input_len; i > input_pos; i--)
            input_buffer[i] = input_buffer[i - 1];

        input_buffer[input_pos] = (char)key;
        input_len++;
        input_pos++;
        input_buffer[input_len] = '\0';

        /* Redraw from inserted character to end of string */
        for (auto i = input_pos - 1; i < input_len; i++)
            terminal_putchar(input_buffer[i]);

        /* Return visual cursor back to insertion position */
        for (auto i = input_len; i > input_pos; i--)
            terminal_cursor_left();
    }
}

void shell_initialize() {
    ps2_set_key_callback(shell_key_handler);
    shell_print_prompt();
}
