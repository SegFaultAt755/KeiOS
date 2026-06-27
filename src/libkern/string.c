#include "libkern/string.h"

uint32_t strlen(const char *str) {
    uint32_t step = 0;
    for (; str[step] != '\0'; step++); 
    
    return step;
}