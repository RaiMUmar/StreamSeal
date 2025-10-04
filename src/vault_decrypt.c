#include "../include/header.h"

/* decrypt_file: legacy whole-file decryption.
   Reads entire ciphertext from `in_path`, validates header, derives a key with Argon2id,
   decrypts with ChaCha20-Poly1305 (IETF), and writes plaintext to `out_path`.
   Returns 0 on success, -1 on error. */
int decrypt_file (const char *in_path, const char *out_path, char *pwd){
    int rc = -1; // default to failure until we complete successfully

    unsigned char *enc = NULL; size_t elen = 0;
    // Read entire input file into memory; on failure, scrub pwd and bail.
    if (read_file(in_path, &enc, &elen) != 0) {  // read ciphertext file
        sodium_memzero(pwd, strlen(pwd)); // scrub password before returning
        return -1; 
    }
    
    // Sanity check: file must at least contain the header.
    if (elen < sizeof(simple_hdr_t)) {
        printf("File too small to be valid.\n");
        sodium_memzero(pwd, strlen(pwd)); // scrub password
        sodium_free(enc); // free ciphertext buffer
        return -1;
    }

    simple_hdr_t hdr;
    memcpy(&hdr, enc, sizeof hdr);  // copy header from start of buffer
    // Validate magic to ensure we're dealing with our format.
    if (memcmp(hdr.magic, MAGIC, sizeof MAGIC) != 0) {
        printf("Bad magic: not our format.\n");
        sodium_memzero(pwd, strlen(pwd)); // scrub password
        sodium_free(enc); // free ciphertext buffer
        return -1;
    }

    unsigned char *cipher = enc + sizeof hdr; // ciphertext payload after header
    size_t clen = elen - sizeof hdr; // ciphertext length without header

    unsigned char key[crypto_aead_chacha20poly1305_ietf_KEYBYTES];
    // Derive encryption key from password and header salt (Argon2id with moderate limits).
    if (crypto_pwhash(
            key, sizeof key,
            pwd, strlen(pwd),
            hdr.salt,
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0) {  // derive key with Argon2id

        printf("crypto_pwhash failed (OOM)\n");
        sodium_memzero(pwd, strlen(pwd)); // scrub password
        sodium_free(enc); // free ciphertext buffer
        return -1;
    }

    sodium_memzero(pwd, strlen(pwd)); // done with password; scrub it

    // Compute maximum possible plaintext length (ciphertext minus AEAD tag).
    size_t max_plen = (clen >= crypto_aead_chacha20poly1305_ietf_ABYTES)
        ? (clen - crypto_aead_chacha20poly1305_ietf_ABYTES)
        : 0;
    unsigned char *plain = (unsigned char*)sodium_malloc(max_plen); // allocate plaintext buffer
    if (!plain) { 
        printf("Malloc Plain Failed\n"); 
        sodium_memzero(key, sizeof key); // scrub key
        sodium_free(enc); // free ciphertext buffer
        return -1; 
    }

    unsigned long long real_plen = 0ULL;
    // Decrypt and authenticate; fails if password is wrong or data was tampered.
    if (crypto_aead_chacha20poly1305_ietf_decrypt(
            plain, &real_plen, 
            NULL,
            cipher, (unsigned long long)clen, 
            NULL, 0, 
            hdr.nonce,
            key) != 0) {  // perform AEAD decryption/verification

        printf("Decryption FAILED (wrong password or tampered file).\n");
        sodium_memzero(key, sizeof key); // scrub key
        sodium_free(enc);  // free ciphertext buffer
        sodium_free(plain); // free plaintext buffer
        return -1;
    }

    // Write the recovered plaintext to the output path.
    if (write_file(out_path, plain, (size_t)real_plen) != 0) {  // write plaintext file
        sodium_memzero(key, sizeof key); // scrub key
        sodium_free(enc);  // free ciphertext buffer
        sodium_free(plain); // free plaintext buffer
        return -1;
    }

    sodium_memzero(key, sizeof key); // scrub key material
    sodium_free(enc); // free ciphertext buffer
    sodium_free(plain); // free plaintext buffer

    rc = 0; // success
    return rc;
}

