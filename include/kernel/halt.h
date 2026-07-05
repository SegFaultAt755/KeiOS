#pragma once

inline void halt() {
    __asm__ volatile ("hlt");
}
