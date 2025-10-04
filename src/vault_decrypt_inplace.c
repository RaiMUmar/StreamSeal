#include "../include/header.h"

/* read_magic: read the first 6 bytes of file `p` into `out` to sniff format.
   Returns 0 on success (read 6 bytes), -1 on error (open/read failure). */
int read_magic(const char *p, unsigned char out[6]){
    FILE *f = fopen(p, "rb"); // open file for binary reading
    // Guard: fail if file couldn't be opened.
    if (!f) return -1;
    size_t n = fread(out, 1, 6, f); // read exactly 6 bytes of header
    fclose(f); // close file handle
    return (n == 6) ? 0 : -1; // success only if 6 bytes were read
}

/* decrypt_inplace: decrypt `in_path` to a sibling path with suffix `wanted_ext`
   (default ".dec"); optionally deletes source if g_delete_on_success is set.
   Supports legacy header (MAGIC) and streamed format autodetection. */
int decrypt_inplace(const char *in_path, char *pwd, const char *wanted_ext) {
    // Guard: validate inputs.
    if (!in_path || !pwd) return -1;

    const char *ext = (wanted_ext && *wanted_ext) ? wanted_ext : ".dec"; // choose suffix (default .dec)
    char out_path[4096];
    if (build_path(in_path, ext, out_path, sizeof out_path) != 0) return -1; // derive output path
    // Guard: avoid identical input/output path; append ".out" if colliding.
    if (strcmp(out_path, in_path) == 0) {
        size_t len = strlen(out_path); // measure current output path
        if (len + 4 + 1 >= sizeof out_path) return -1; // capacity check for ".out" + NUL
        strcat(out_path, ".out"); // disambiguate output filename
    }

    unsigned char m[6];
    int rc;
    // Branch: legacy SIMPL1 format uses old decryptor; otherwise use streaming.
    if (read_magic(in_path, m) == 0 && memcmp(m, MAGIC, 6) == 0)
        rc = decrypt_file(in_path, out_path, pwd); // legacy decrypt (AEAD ChaCha20-Poly1305)
    else
        rc = decrypt_file_stream(in_path, out_path, pwd); // streamed decrypt (secretstream)

    // Post-decrypt: optionally delete source on success.
    if (rc == 0) {
        // If opt-in deletion is enabled, try to remove the input file.
        if (g_delete_on_success) {   
            if (safe_delete(in_path) != 0) // unlink source file
                fprintf(stderr, "Warning: could not delete original file: %s\n", in_path); // warn if deletion fails
        }
        return 0; // success
    }
    return -1; // failure
}


