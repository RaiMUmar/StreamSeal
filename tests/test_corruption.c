#include "../include/header.h"

/* write_file_simple: create/overwrite file `p` with string `s` (binary mode). */
static void write_file_simple(const char *p, const char *s){
    FILE *f = fopen(p,"wb"); assert(f);              // open destination file
    fwrite(s,1,strlen(s),f);                         // write entire string
    fclose(f);                                       // close file
}

/* file_copy: copy file from `src` to `dst` using project I/O helpers.
   Returns 0 on success, -1 on failure. */
static int file_copy(const char *src, const char *dst){
    unsigned char *buf = NULL; size_t len = 0;       // init buffer/length
    if (read_file(src, &buf, &len) != 0) return -1;  // read source fully
    if (write_file(dst, buf, len) != 0){             // write to destination
        sodium_free(buf);                            // free on failure
        return -1;
    }
    sodium_free(buf);                                // free on success
    return 0;
}

/* mutate_file_byte: flip one byte at absolute offset in `path`.
   If offset is beyond EOF, seeks to last byte instead. Returns 0 on success, -1 on failure. */
static int mutate_file_byte(const char *path, long offset){
    FILE *f = fopen(path, "r+b");                    // open for read/write
    if (!f) return -1;                               // fail if can't open
    if (fseek(f, 0, SEEK_END) != 0){ fclose(f); return -1; } // seek to end
    long sz = ftell(f);                              // measure file size
    if (sz <= 0){ fclose(f); return -1; }            // nothing to flip
    if (offset >= sz) offset = sz - 1;               // clamp to last byte

    if (fseek(f, offset, SEEK_SET) != 0){ fclose(f); return -1; } // seek to target
    unsigned char b = 0;
    if (fread(&b, 1, 1, f) != 1){ fclose(f); return -1; }        // read byte
    if (fseek(f, offset, SEEK_SET) != 0){ fclose(f); return -1; } // rewind 1 byte
    b ^= 0x01;                                       // flip lowest bit
    if (fwrite(&b, 1, 1, f) != 1){ fclose(f); return -1; }       // write mutated byte
    fclose(f);                                       // close file
    return 0;
}

/* file_size: return the size (bytes) of a regular file at `path`, or -1 on error. */
static long file_size(const char *path){
    struct stat st;                                  // stat buffer
    if (stat(path, &st) != 0) return -1;             // stat failure
    return (long)st.st_size;                         // return size
}

/* main: corruption tests for streamed format, with quiet logs.
   - Confirms valid encrypt/decrypt roundtrip.
   - Corrupts header → decrypt must fail (rc=-1) and produce empty output.
   - Corrupts payload (single-chunk) → decrypt must fail (rc=-1) and produce empty output.
   Stderr from the expected failures is suppressed to keep test output clean. */
int main(void){
    assert(sodium_init() >= 0);                      // libsodium must initialize

    char dir[] = "/tmp/ss-corrupt-XXXXXX";
    assert(mkdtemp(dir) && "mkdtemp failed");        // create temp directory

    // Build file paths used in tests.
    char plain[512], enc[512], dec_ok[512], enc_bad_hdr[512], dec_bad_hdr[512], enc_bad_ct[512], dec_bad_ct[512];
    snprintf(plain,       sizeof plain, "%s/plain.txt",        dir); // plaintext
    snprintf(enc,         sizeof enc,   "%s/plain.enc",        dir); // good ciphertext
    snprintf(dec_ok,      sizeof dec_ok,"%s/plain.dec",        dir); // good decrypt output
    snprintf(enc_bad_hdr, sizeof enc_bad_hdr, "%s/bad_hdr.enc", dir); // header-corrupted ciphertext
    snprintf(dec_bad_hdr, sizeof dec_bad_hdr, "%s/bad_hdr.dec", dir); // output for bad header
    snprintf(enc_bad_ct,  sizeof enc_bad_ct,  "%s/bad_ct.enc",  dir); // payload-corrupted ciphertext
    snprintf(dec_bad_ct,  sizeof dec_bad_ct,  "%s/bad_ct.dec",  dir); // output for bad payload

    // Prepare small plaintext (single-chunk) so any corruption aborts before writing data.
    const char *msg = "corrupt_me_plz";              // <= STREAM_CHUNK to keep single chunk
    write_file_simple(plain, msg);                   // seed plaintext

    // Encrypt (streaming) -> enc
    char pw[] = "p@ss";                              // test password
    assert(encrypt_file_stream(plain, enc, pw) == 0); // create streaming ciphertext

    // Sanity: valid decrypt should succeed and match original content.
    char pw_ok[] = "p@ss";                           // correct password
    assert(decrypt_file_stream(enc, dec_ok, pw_ok) == 0); // decrypt successfully
    long ok_sz = file_size(dec_ok);                  // measure decrypted size
    assert(ok_sz == (long)strlen(msg));              // size must match plaintext
    FILE *f = fopen(dec_ok,"rb"); assert(f);         // open decrypted file
    char buf[64] = {0};                              // buffer for readback
    fread(buf,1,sizeof buf,f); fclose(f);            // read contents
    assert(strcmp(buf, msg) == 0);                   // contents must match

    // --- Test 1: Corrupt header magic (quiet expected error logs) ---
    assert(file_copy(enc, enc_bad_hdr) == 0);        // duplicate ciphertext to mutate header
    assert(mutate_file_byte(enc_bad_hdr, 0) == 0);   // flip first byte (magic)

    int saved_err_1 = dup(STDERR_FILENO);            // save current stderr
    FILE *devnull_1 = fopen("/dev/null","w");        // open /dev/null sink
    if (devnull_1) dup2(fileno(devnull_1), STDERR_FILENO); // redirect stderr → /dev/null

    char pw1[] = "p@ss";                             // correct password
    int rc1 = decrypt_file_stream(enc_bad_hdr, dec_bad_hdr, pw1); // attempt decrypt (should fail)

    fflush(stderr);                                   // flush redirected stream
    if (saved_err_1 >= 0) { dup2(saved_err_1, STDERR_FILENO); close(saved_err_1); } // restore stderr
    if (devnull_1) fclose(devnull_1);                // close sink
    assert(rc1 == -1);                                // decryption must fail on bad header

    long bad_hdr_sz = file_size(dec_bad_hdr);        // check output size
    assert(bad_hdr_sz == 0 || bad_hdr_sz == -1);     // output should be empty or absent

    // --- Test 2: Corrupt payload byte (single chunk, quiet expected error logs) ---
    assert(file_copy(enc, enc_bad_ct) == 0);         // duplicate ciphertext to mutate payload
    assert(mutate_file_byte(enc_bad_ct, (long)sizeof(stream_hdr_t) + 5) == 0); // flip a byte in first ciphertext chunk

    int saved_err_2 = dup(STDERR_FILENO);            // save stderr again
    FILE *devnull_2 = fopen("/dev/null","w");        // open /dev/null sink
    if (devnull_2) dup2(fileno(devnull_2), STDERR_FILENO); // redirect stderr → /dev/null

    char pw2[] = "p@ss";                             // correct password
    int rc2 = decrypt_file_stream(enc_bad_ct, dec_bad_ct, pw2); // attempt decrypt (should fail)

    fflush(stderr);                                   // flush redirected stream
    if (saved_err_2 >= 0) { dup2(saved_err_2, STDERR_FILENO); close(saved_err_2); } // restore stderr
    if (devnull_2) fclose(devnull_2);                // close sink
    assert(rc2 == -1);                                // decryption must fail on corrupted payload

    long bad_ct_sz = file_size(dec_bad_ct);          // check output size
    assert(bad_ct_sz == 0 || bad_ct_sz == -1);       // output should be empty or absent

    // Optional cleanup of temp artifacts.
    unlink(plain); unlink(enc); unlink(dec_ok);       // remove files if present
    unlink(enc_bad_hdr); unlink(dec_bad_hdr);         // remove corrupted header files
    unlink(enc_bad_ct);  unlink(dec_bad_ct);          // remove corrupted payload files
    rmdir(dir);                                       // remove temp directory

    return 0;                                         // all corruption tests passed
}

