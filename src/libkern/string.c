#include "libkern/string.h"

uint32_t strlen(const char *str) {
    uint32_t step = 0;
    for (; str[step] != '\0'; step++); 
    
    return step;
}

int uvalue_to_str(char *buffer, unsigned int value, int base, int min_width, int zero_padding) {
    int i = 0;
    
    if (value == 0) {
        buffer[i++] = '0';
    } else {
        while (value > 0) {
            int rem = value % base;
            buffer[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
            value /= base;
        }
    }

    /* Apply zero padding */
    while (i < min_width && i < 32 && zero_padding) {
        buffer[i++] = '0';
    }

    /* Reverse the buffer */
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }

    return i; /* Returns length of the string */
}
