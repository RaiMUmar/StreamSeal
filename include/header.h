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


int main(int argc, char **argv);
int decrypt_file (const char *in_path, const char *out_path);
static int encrypt_file (const char *in_path, const char *out_path);
int read_file (const char*path, unsigned char **buff, size_t *len);
int write_file(const char *path, const unsigned char *buf, size_t len);
int prompt_password(const char *label, char *out, size_t outsz, int confirm);
static int init_user();
int login_user();
void print_hex (const char *label, const unsigned char *buf, size_t len);
static void usage(const char *prog);