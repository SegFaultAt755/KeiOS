#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

struct rsdp;

#define ACPI_SCI_EN (1 << 0)

struct [[gnu::packed]] acpi_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

struct [[gnu::packed]] fadt {
    struct acpi_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved1;
    uint8_t  preferred_pm_profile;
    uint16_t sci_int;            /* System Control Interrupt vector */
    uint32_t smi_cmd;            /* SMI command port */
    uint8_t  acpi_enable;        /* Value written to smi_cmd to enable ACPI */
    uint8_t  acpi_disable;       /* Value written to smi_cmd to disable ACPI */
    uint8_t  s4bios_req;
    uint8_t  pstate_cnt;
    uint32_t pm1a_evt_blk;       /* PM1a event block I/O port */
    uint32_t pm1b_evt_blk;       /* PM1b event block I/O port */
    uint32_t pm1a_cnt_blk;       /* PM1a control block I/O port */
    uint32_t pm1b_cnt_blk;       /* PM1b control block I/O port */
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;         /* ACPI power-management timer port */
};

struct [[gnu::packed]] dsdt {
    struct acpi_header header;
    uint8_t aml_code[];
};

extern struct fadt *global_fadt;
extern struct dsdt *global_dsdt;

#define ACPI_SLP_EN (1 << 13)

void acpi_shutdown(struct fadt *fadt, struct dsdt *dsdt);

struct dsdt *parse_dsdt(struct fadt *fadt);
struct acpi_header *find_fadt(struct acpi_header *table, bool is_xsdt);
void enable_acpi_mode(struct fadt *fadt);
struct fadt *acpi_init(struct rsdp *rsdp);
