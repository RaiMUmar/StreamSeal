#include "../include/header.h"

/* encrypt_file: legacy whole-file encryption.
   Reads plaintext from `in_path`, derives a key with Argon2id, encrypts with
   ChaCha20-Poly1305 (IETF), prepends a simple header, and writes to `out_path`.
   Returns 0 on success, -1 on error. */
int encrypt_file (const char *in_path, const char *out_path, char *pwd) {
    int rc = -1; // default to failure until we complete successfully

    unsigned char *plain = NULL; size_t plen = 0;
    if (read_file (in_path, &plain, &plen) != 0){ // read entire plaintext file into memory
        sodium_memzero(pwd, strlen(pwd)); // scrub password on failure
        return -1;
    }

    simple_hdr_t hdr;
    memcpy(hdr.magic, MAGIC, sizeof(MAGIC)); // set magic identifier
    randombytes_buf(hdr.salt, sizeof(hdr.salt)); // generate random KDF salt
    randombytes_buf(hdr.nonce, sizeof(hdr.nonce)); // generate AEAD nonce

    unsigned char key[crypto_aead_chacha20poly1305_ietf_KEYBYTES];
    // Derive key from password using Argon2id with moderate limits.
    if (crypto_pwhash(
            key, sizeof(key),
            pwd, strlen(pwd),
            hdr.salt,
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0){ // key derivation (Argon2id)

        printf("Encryption Failed!\n");
        sodium_memzero(pwd, strlen(pwd)); // scrub password
        sodium_free(plain); // free plaintext buffer
        return -1;
    }

    sodium_memzero(pwd, strlen(pwd)); // done with password; scrub it

    size_t clen = plen + crypto_aead_chacha20poly1305_ietf_ABYTES; // ciphertext = plaintext + tag
    unsigned char *cipher = sodium_malloc(clen); // allocate ciphertext buffer
    if (!cipher){
        printf("Malloc Failure!\n");
        sodium_memzero(key, sizeof(key)); // scrub derived key
        return -1;
    }
    
    unsigned long long real_clen = 0ULL;
    // Encrypt and authenticate plaintext using AEAD (no AAD in this legacy format).
    if (crypto_aead_chacha20poly1305_ietf_encrypt(
            cipher, &real_clen,
            plain, plen,
            NULL, 0,
            NULL,
            hdr.nonce,
            key) != 0){ // perform AEAD encryption

        printf("Encryption Failed!\n");
        sodium_memzero(key, sizeof(key)); // scrub key
        sodium_free(plain); // free plaintext buffer
        sodium_free(cipher); // free ciphertext buffer
        return -1;
    }

    size_t total_out = sizeof(hdr) + real_clen; // header + ciphertext size
    unsigned char *outbuf = sodium_malloc(total_out); // allocate output buffer
    if (!outbuf){
        printf("Malloc Failure!\n");
        sodium_memzero(key, sizeof(key)); // scrub key
        return -1;
    }

    memcpy(outbuf, &hdr, sizeof(hdr)); // write header at start of output
    memcpy(outbuf + sizeof(hdr), cipher, real_clen); // append ciphertext after header

    // Write the combined header+ciphertext to destination path.
    if (write_file(out_path, outbuf, total_out) != 0){ // write output file
        sodium_memzero(key, sizeof(key)); // scrub key
        sodium_free(plain); // free plaintext buffer
        sodium_free(cipher); // free ciphertext buffer
        sodium_free(outbuf); // free output buffer
        return -1;
    }

    sodium_memzero(key, sizeof key); // scrub key material
    sodium_free(plain); // free plaintext buffer
    sodium_free(cipher); // free ciphertext buffer
    sodium_free(outbuf); // free output buffer

    rc = 0; // success
    return rc;
}

