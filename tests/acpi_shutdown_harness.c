#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "../src/kernel/acpi/shutdown.c"

static uint32_t write_count;
static uint16_t last_port;
static uint16_t last_value;
static uint32_t log_count;

void outb(uint16_t port, uint8_t value) {
    (void)port;
    (void)value;
}

void outw(uint16_t port, uint16_t value) {
    write_count++;
    last_port = port;
    last_value = value;
}

void qemu_printf(enum qemu_category category, enum qemu_level level, const char *format, ...) {
    (void)category;
    (void)level;
    (void)format;
    log_count++;
}

static struct dsdt *make_dsdt(uint8_t *storage, const uint8_t *aml, size_t aml_len) {
    struct dsdt *dsdt = (struct dsdt *)storage;
    dsdt->header.length = sizeof(struct acpi_header) + (uint32_t)aml_len;
    if (aml_len != 0) {
        memcpy(dsdt->aml_code, aml, aml_len);
    }
    return dsdt;
}

static void reset_observations(void) {
    write_count = 0;
    last_port = 0;
    last_value = 0;
    log_count = 0;
}

int main(void) {
    uint8_t storage[sizeof(struct dsdt) + 32] = {0};
    struct fadt fadt = {.pm1a_cnt_blk = 0x604, .pm1b_cnt_blk = 0x608};

    struct dsdt *dsdt = make_dsdt(storage, NULL, 0);
    dsdt->header.length = sizeof(struct acpi_header) - 1;
    acpi_shutdown(&fadt, dsdt);
    assert(write_count == 0 && log_count == 1);

    const uint8_t truncated_pkg_length[] = {'_', 'S', '5', '_', 0x12, 0x41};
    reset_observations();
    acpi_shutdown(&fadt, make_dsdt(storage, truncated_pkg_length, sizeof(truncated_pkg_length)));
    assert(write_count == 0 && log_count == 1);

    const uint8_t oversized_package[] = {'_', 'S', '5', '_', 0x12, 0x20, 0x02, 0x0A, 0x05};
    reset_observations();
    acpi_shutdown(&fadt, make_dsdt(storage, oversized_package, sizeof(oversized_package)));
    assert(write_count == 0 && log_count == 1);

    const uint8_t invalid_sleep_type[] = {'_', 'S', '5', '_', 0x12, 0x06, 0x02, 0x0A, 0x08, 0x0A, 0x05};
    reset_observations();
    acpi_shutdown(&fadt, make_dsdt(storage, invalid_sleep_type, sizeof(invalid_sleep_type)));
    assert(write_count == 0 && log_count == 1);

    const uint8_t valid_s5[] = {'_', 'S', '5', '_', 0x12, 0x06, 0x02, 0x0A, 0x05, 0x0A, 0x06};
    reset_observations();
    acpi_shutdown(&fadt, make_dsdt(storage, valid_s5, sizeof(valid_s5)));
    assert(write_count == 2);
    assert(last_port == fadt.pm1b_cnt_blk);
    assert(last_value == ((6U << 10) | ACPI_SLP_EN));

    reset_observations();
    fadt.pm1a_cnt_blk = UINT16_MAX + 1U;
    acpi_shutdown(&fadt, make_dsdt(storage, valid_s5, sizeof(valid_s5)));
    assert(write_count == 0 && log_count == 1);
    return 0;
}
