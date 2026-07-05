#ifndef __HALT_H__
#define __HALT_H__

inline void halt() {
    __asm__ volatile ("hlt");
}

#endif