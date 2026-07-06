#include "kernel/memory.h"
#include "kernel/qemu.h"
#include "libkern/stdio.h"
#include <stdint.h>
#include <stddef.h>

void memory_initialize(MultibootInfo *boot_info) {
    for (size_t i = 0; i < boot_info -> mmap_length; i += sizeof(MultibootMmapEntry)) {
        MultibootMmapEntry *mmnt = (MultibootMmapEntry*)(boot_info -> mmap_address + i); /* Parse and get mmap table */
        
        qemu_printf(QEMU_LOG_INFO,
            "Memory info %d:\n\t\tLow address: %x\n\t\tHigh address: %x\n\t\tLow length: %x\n\t\tHigh length: %x\n\t\tSize: %x\n\t\tType: %d\n",
            i, mmnt -> address_lower, mmnt -> address_upper, mmnt -> length_lower, mmnt -> length_upper, mmnt -> size, mmnt -> type
        );
    }
}