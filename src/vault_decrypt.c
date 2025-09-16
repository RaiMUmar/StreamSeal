#include "../include/header.h"


int decrypt_file (const char *in_path, const char *out_path, char *pwd){
    int rc = -1; // Assume Failure Until Success

    unsigned char *enc = NULL; size_t elen = 0;
    if (read_file(in_path, &enc, &elen) != 1) { 
        sodium_memzero(pwd, strlen(pwd)); 
        return -1; 
    }
    
    if (elen < sizeof(simple_hdr_t)) {
        printf("File too small to be valid.\n");
        sodium_memzero(pwd, strlen(pwd));
        sodium_free(enc);
        return -1;
    }

    simple_hdr_t hdr;
    memcpy(&hdr, enc, sizeof hdr); 
    if (memcmp(hdr.magic, MAGIC, sizeof MAGIC) != 0) {
        printf("Bad magic: not our format.\n");
        sodium_memzero(pwd, strlen(pwd));
        sodium_free(enc);
        return -1;
    }

    unsigned char *cipher = enc + sizeof hdr;
    size_t clen = elen - sizeof hdr;

    unsigned char key[crypto_aead_chacha20poly1305_ietf_KEYBYTES];
    if (crypto_pwhash(
            key, sizeof key,
            pwd, strlen(pwd),
            hdr.salt,
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0) {

        printf("crypto_pwhash failed (OOM)\n");
        sodium_memzero(pwd, strlen(pwd));
        sodium_free(enc);
        return -1;
    }

    sodium_memzero(pwd, strlen(pwd)); // done with password

    size_t max_plen = (clen >= crypto_aead_chacha20poly1305_ietf_ABYTES)
        ? (clen - crypto_aead_chacha20poly1305_ietf_ABYTES)
        : 0;
    unsigned char *plain = (unsigned char*)sodium_malloc(max_plen);
    if (!plain) { 
        printf("Malloc Plain Failed\n"); 
        sodium_memzero(key, sizeof key); 
        sodium_free(enc); 
        return -1; 
    }


    unsigned long long real_plen = 0ULL;
    if (crypto_aead_chacha20poly1305_ietf_decrypt(
            plain, &real_plen, 
            NULL,
            cipher, (unsigned long long)clen, 
            NULL, 0, 
            hdr.nonce,
            key) != 0) { 

        printf("Decryption FAILED (wrong password or tampered file).\n");
        sodium_memzero(key, sizeof key);
        sodium_free(enc); 
        sodium_free(plain);
        return -1;
    }

    if (write_file(out_path, plain, (size_t)real_plen) != 1) {
        sodium_memzero(key, sizeof key);
        sodium_free(enc); 
        sodium_free(plain);
        return -1;
    }

    sodium_memzero(key, sizeof key); // scrub key material
    sodium_free(enc);
    sodium_free(plain);

    rc = 1; // success
    return rc;
}
