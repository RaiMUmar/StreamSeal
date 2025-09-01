#include <stdio.h>
#include <sodium.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <string.h>

static const uint8_t MAGIC[6] = { 'S','I','M','P','L','1' };

typedef struct {
    uint8_t  magic[6];
    unsigned char salt[16];
    unsigned char nonce[12];
} simple_hdr_t;