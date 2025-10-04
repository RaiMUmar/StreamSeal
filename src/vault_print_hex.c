#include "../include/header.h"

/* print_hex: print a label followed by `len` bytes of `buf` as lowercase hex. */
void print_hex (const char *label, const unsigned char *buf, size_t len) {
    printf("%s", label); // print the provided label as-is
    // Loop: emit each byte as two hex digits (zero-padded).
    for (size_t i = 0; i < len; i++){
        printf("%02x", buf[i]); // print one byte in hex
    }
    printf("\n"); // terminate the line
}

