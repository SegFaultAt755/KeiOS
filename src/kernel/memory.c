#include "kernel/memory.h"
#include "libkern/stdio.h"
#include <stdint.h>
#include <stddef.h>

void memory_initialize(MultibootInfo *boot_info) {
    for (size_t i = 0; i < boot_info -> mmap_length; i += sizeof(MultibootMmapEntry)) {
        MultibootMmapEntry *mmnt = (MultibootMmapEntry*)(boot_info -> mmap_address + i); /* Parse and get mmap table */
        
        kprintf(LOG_DEBUG,
            "Memory info: Low address: %x | High address: %x | Low length: %x | High length: %x | Size: %x | Type: %d\n",
            mmnt -> address_lower, mmnt -> address_upper, mmnt -> length_lower, mmnt -> length_upper, mmnt -> size, mmnt -> type
        );
    }
}