#include <stdio.h>
#include <sodium.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <string.h>
#include <errno.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <termios.h>
#include <fcntl.h>

static const uint8_t MAGIC[6] = { 'S','I','M','P','L','1' };

typedef struct {
    uint8_t  magic[6];
    unsigned char salt[16];
    unsigned char nonce[12];
} simple_hdr_t;

typedef int (*encrypt_func)(const char*, char*, const char*);


int main(int argc, char **argv);
int decrypt_file (const char *in_path, const char *out_path, char *pwd);
int encrypt_file (const char *in_path, const char *out_path, char *pwd);
int read_file (const char*path, unsigned char **buff, size_t *len);
int write_file(const char *path, const unsigned char *buf, size_t len);
int prompt_password(const char *label, char *out, size_t outsz, int confirm);
int init_user(void);
int login_user(char *pwd);
void print_hex (const char *label, const unsigned char *buf, size_t len);
void usage(const char *prog);
int user_created(const char *path);
int path_handler(encrypt_func f,const char *path, char *pwd, const char *suffix);
int decrypt_inplace(const char *in_path, char *pwd, const char *wanted_ext);
int encrypt_inplace(const char *in_path, char *pwd, const char *garbage);
int build_path(const char *in_path, const char *suffix, char *out_path, size_t out_sz);
int safe_delete(const char *path);
int ends_with(const char *s, const char *suffix);
const char *base_name(const char *path);
