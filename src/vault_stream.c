#include "../include/header.h"

/* write_all: write exactly n bytes from buf to stream f.
   Returns 0 on success (all bytes written), -1 on short write/error. */
/* Read Any File In Binary Form and Store Its Data Into buff Variable */
static int write_all(FILE *f, const void *buf, size_t n){
    return fwrite(buf, 1, n, f) == n ? 0 : -1; // attempt full write and check count
}
/* read_all: read exactly n bytes from stream f into buf.
   Returns 0 on success (all bytes read), -1 on short read/error. */
static int read_all(FILE *f, void *buf, size_t n){
    return fread(buf, 1, n, f) == n ? 0 : -1; // attempt full read and check count
}

/* encrypt_file_stream: streamed encryption using libsodium secretstream.
   - Derives a key via Argon2id (from pwd + salt in header)
   - Binds header fields as AAD
   - Streams chunks with constant memory and final tag
   Writes result to out_path. Returns 0 on success, -1 on failure. */
int encrypt_file_stream(const char *in_path, const char *out_path, char *pwd){
    FILE *in = fopen(in_path, "rb"); // open input for reading
    if (!in){ perror("fopen in"); return -1; } // fail if cannot open
    FILE *out = fopen(out_path, "wb"); // open output for writing
    if (!out){ perror("fopen out"); fclose(in); return -1; } // clean up input on failure

    stream_hdr_t hdr;
    memcpy(hdr.magic, STREAM_MAGIC, sizeof(STREAM_MAGIC)); // set streaming magic
    hdr.version       = STREAMSEAL_VERSION; // set format version
    hdr.kdf_mem_kib   = (uint32_t)(crypto_pwhash_MEMLIMIT_MODERATE / 1024); // record KDF mem
    hdr.kdf_opslimit  = (uint32_t) crypto_pwhash_OPSLIMIT_MODERATE; // record KDF ops
    randombytes_buf(hdr.salt, sizeof hdr.salt); // generate salt

    unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES];
    if (crypto_pwhash(key, sizeof key,
                      pwd, strlen(pwd),
                      hdr.salt,
                      crypto_pwhash_OPSLIMIT_MODERATE,
                      crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0){
        fprintf(stderr, "KDF failed\n");
        fclose(in); fclose(out); // close streams on failure
        return -1;
    }
    sodium_memzero(pwd, strlen(pwd)); /* done with password */ // scrub pwd promptly

    crypto_secretstream_xchacha20poly1305_state st;
    if (crypto_secretstream_xchacha20poly1305_init_push(&st, hdr.ss_header, key) != 0){
        fprintf(stderr, "secretstream init_push failed\n");
        sodium_memzero(key, sizeof key); // scrub key on failure
        fclose(in); fclose(out); // close streams
        return -1;
    }

    /* AAD = header prefix (binds magic+version+KDF params+salt) */
    const unsigned char *aad = (const unsigned char *)&hdr; // point to header bytes
    const size_t aad_len = offsetof(stream_hdr_t, ss_header); // AAD excludes ss_header

    /* write full header first (includes ss_header) */
    if (write_all(out, &hdr, sizeof hdr) != 0){
        perror("write header");
        sodium_memzero(key, sizeof key); // scrub key
        fclose(in); fclose(out); // close streams
        return -1;
    }

    unsigned char inbuf[STREAM_CHUNK]; // chunk buffer for plaintext
    unsigned char outbuf[STREAM_CHUNK + crypto_secretstream_xchacha20poly1305_ABYTES]; // ciphertext chunk
    int rc = -1; // default to failure

    // Stream loop: read plaintext chunks, push encrypted chunks.
    for (;;) {
        size_t n = fread(inbuf, 1, sizeof inbuf, in); // read next chunk
        if (ferror(in)){ perror("fread"); break; } // stop on read error

        unsigned char tag = feof(in) ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0; // mark final chunk
        unsigned long long clen = 0ULL;

        if (crypto_secretstream_xchacha20poly1305_push(
                &st, outbuf, &clen, inbuf, n, aad, aad_len, tag) != 0){
            fprintf(stderr, "crypto_secretstream push failed\n");
            break;
        }
        if (write_all(out, outbuf, (size_t)clen) != 0){
            perror("write chunk");
            break;
        }
        if (feof(in)){ rc = 0; break; } // done after writing final chunk
    }

    sodium_memzero(key, sizeof key); // scrub key
    fclose(in); // close input
    if (fclose(out) != 0) rc = -1; // close output and propagate error if any
    return rc; // 0 on success, -1 on failure
}

/* decrypt_file_stream: streamed decryption for secretstream format.
   - Reads and validates header (magic/version)
   - KDF using recorded params from header
   - Binds same header bytes as AAD
   - Pulls chunks until FINAL tag
   Writes plaintext to out_path. Returns 0 on success, -1 on failure. */
int decrypt_file_stream(const char *in_path, const char *out_path, char *pwd){
    FILE *in = fopen(in_path, "rb"); // open input for reading
    if (!in){ perror("fopen in"); return -1; } // fail if cannot open
    FILE *out = fopen(out_path, "wb"); // open output for writing
    if (!out){ perror("fopen out"); fclose(in); return -1; } // clean up input on failure

    stream_hdr_t hdr;
    if (read_all(in, &hdr, sizeof hdr) != 0){
        fprintf(stderr, "short or missing header\n");
        fclose(in); fclose(out); // close streams
        return -1;
    }
    if (memcmp(hdr.magic, STREAM_MAGIC, sizeof(STREAM_MAGIC)) != 0 ||
        hdr.version != STREAMSEAL_VERSION){
        fprintf(stderr, "bad magic/version (not StreamSeal)\n");
        fclose(in); fclose(out); // close streams
        return -1;
    }

    unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES];
    if (crypto_pwhash(key, sizeof key,
                      pwd, strlen(pwd),
                      hdr.salt,
                      (unsigned long long)hdr.kdf_opslimit,
                      (size_t)hdr.kdf_mem_kib * 1024ULL,
                      crypto_pwhash_ALG_ARGON2ID13) != 0){
        fprintf(stderr, "KDF failed\n");
        fclose(in); fclose(out); // close streams
        return -1;
    }
    sodium_memzero(pwd, strlen(pwd)); /* done with password */ // scrub pwd promptly

    crypto_secretstream_xchacha20poly1305_state st;
    if (crypto_secretstream_xchacha20poly1305_init_pull(&st, hdr.ss_header, key) != 0){
        fprintf(stderr, "secretstream init_pull failed\n");
        sodium_memzero(key, sizeof key); // scrub key
        fclose(in); fclose(out); // close streams
        return -1;
    }

    const unsigned char *aad = (const unsigned char *)&hdr; // same header AAD as on encrypt
    const size_t aad_len = offsetof(stream_hdr_t, ss_header); // AAD excludes ss_header

    unsigned char inbuf[STREAM_CHUNK + crypto_secretstream_xchacha20poly1305_ABYTES]; // ciphertext chunk
    unsigned char outbuf[STREAM_CHUNK]; // plaintext chunk
    int rc = -1; // default to failure

    // Stream loop: read encrypted chunks, pull into plaintext and write out.
    for (;;) {
        size_t n = fread(inbuf, 1, sizeof inbuf, in); // read next ciphertext chunk
        if (n == 0){
            if (feof(in)) { rc = 0; break; } /* clean EOF only if FINAL already seen */ // allow EOF if final was processed
            perror("fread"); break; // read error
        }

        unsigned long long plen = 0ULL;
        unsigned char tag = 0;
        if (crypto_secretstream_xchacha20poly1305_pull(
                &st, outbuf, &plen, &tag, inbuf, (unsigned long long)n, aad, aad_len) != 0){
            fprintf(stderr, "decryption failed (wrong password or corrupted data)\n");
            break;
        }
        if (write_all(out, outbuf, (size_t)plen) != 0){
            perror("write chunk");
            break;
        }
        if (tag & crypto_secretstream_xchacha20poly1305_TAG_FINAL){
            /* Next read should be EOF; we accept it and stop */
            rc = 0; // mark success after final tag
            break;
        }
    }

    sodium_memzero(key, sizeof key); // scrub key
    fclose(in); // close input
    if (fclose(out) != 0) rc = -1; // close output, propagate error if close fails
    return rc; // 0 on success, -1 on failure
}

