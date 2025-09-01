#include "../include/header.h"

static int encrypt_file (const char *in_path, const char *out_path) {
    int rc = -1; // Assume Failure Unitil Success

    char pwd [1024];
    if (prompt_password("Enter Password To Encrypt: ", pwd, sizeof(pwd),0) != 1){
        return -1;
    }

    unsigned char *plain = NULL; size_t plen = 0;
    if (read_file (in_path, &plain, &plen) != 1){
        sodium_memzero(pwd, sizeof(pwd));
        return -1;
    }

    simple_hdr_t hdr;
    memcpy(hdr.magic, MAGIC, sizeof(MAGIC));
    randombytes_buf(hdr.salt, sizeof(hdr.salt));
    randombytes_buf(hdr.nonce, sizeof(hdr.nonce));

    unsigned char key[crypto_aead_chacha20poly1305_ietf_KEYBYTES];
    if (crypto_pwhash(
            key, sizeof(key),
            pwd, strlen(pwd),
            hdr.salt,
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0){

        printf("Encryption Failed!\n");
        sodium_memzero(pwd, sizeof(pwd));
        sodium_free(plain);
        return -1;
    }

    sodium_memzero(pwd, sizeof(pwd));

    size_t clen = plen + crypto_aead_chacha20poly1305_ietf_ABYTES;
    unsigned char *cipher = sodium_malloc(clen);
    if (!cipher){
        printf("Malloc Failure!\n");
        sodium_memzero(key, sizeof(key));
        return -1;
    }
    
    unsigned long long real_clen = 0ULL;
    if (crypto_aead_chacha20poly1305_ietf_encrypt(
            cipher, &real_clen,
            plain, plen,
            NULL, 0,
            NULL,
            hdr.nonce,
            key) != 0){
        
        printf("Encryption Failed!\n");
        sodium_memzero(key, sizeof(key));
        sodium_free(plain);
        sodium_free(cipher);
        return -1;
    }

    size_t total_out = sizeof(hdr) + real_clen;
    unsigned char *outbuf = sodium_malloc(total_out);
    if (!outbuf){
        printf("Malloc Failure!\n");
        sodium_memzero(key, sizeof(key));
        return -1;
    }

    memcpy(outbuf, &hdr, sizeof(hdr));
    memcpy(outbuf + sizeof(hdr), cipher, real_clen);

    if (write_file(out_path, outbuf, total_out) != 1){
        sodium_memzero(key, sizeof(key));
        sodium_free(plain);
        sodium_free(cipher);
        sodium_free(outbuf);
        return -1;
    }

    sodium_memzero(key, sizeof key);
    sodium_free(plain);
    sodium_free(cipher);
    sodium_free(outbuf);

    rc = 1;
    return rc;

    return 1;
}