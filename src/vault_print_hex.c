#include "../include/header.h"

void print_hex (const char *label, const unsigned char *buf, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; i++){
        printf("%02x", buf[i]);
    }
    printf("\n");
}
