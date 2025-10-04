#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

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
#include <limits.h>
#include <unistd.h>
#include <stddef.h> 
#include <assert.h>

static const uint8_t MAGIC[6] = { 'S','I','M','P','L','1' };

typedef struct {
    uint8_t  magic[6];
    unsigned char salt[16];
    unsigned char nonce[12];
} simple_hdr_t;

#define STREAMSEAL_VERSION 1
#define STREAM_CHUNK (64 * 1024)
static const uint8_t STREAM_MAGIC[6] = { 'S','E','A','L','v','1' };

typedef struct {
    uint8_t  magic[6];   /* "SEALv1" */
    uint16_t version;    /* STREAMSEAL_VERSION */
    uint32_t kdf_mem_kib;   /* Argon2id mem limit used (KiB) */
    uint32_t kdf_opslimit;  /* Argon2id ops limit used */
    unsigned char salt[16]; /* Argon2id salt */
    unsigned char ss_header[crypto_secretstream_xchacha20poly1305_HEADERBYTES]; /* secretstream header */
} stream_hdr_t;

typedef int (*encrypt_func)(const char*, char*, const char*);


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
int write_file_atomic_0600(const char *path, const unsigned char *buf, size_t len);
int encrypt_file_stream(const char *in_path, const char *out_path, char *pwd);
int decrypt_file_stream(const char *in_path, const char *out_path, char *pwd);
int read_magic(const char *p, unsigned char out[6]);


extern int g_delete_on_success;

#ifdef __cplusplus
}
#endif

#endif
