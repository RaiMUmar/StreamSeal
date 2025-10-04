#include "../include/header.h"

/* write_file_simple: create/overwrite file `p` with string `s` (binary mode). */
static void write_file_simple(const char *p, const char *s){
    FILE *f = fopen(p,"wb"); assert(f);              // open destination file
    fwrite(s,1,strlen(s),f);                         // write entire string
    fclose(f);                                       // close file
}

/* main: end-to-end roundtrip test for encrypt_inplace/decrypt_inplace.
   Verifies delete-on-success, file presence, and final plaintext integrity. */
int main(void){
    assert(sodium_init() >= 0);                      // libsodium must initialize

    g_delete_on_success = 1;                         // opt-in: delete source files on success

    char dir[] = "/tmp/vault-XXXXXX";
    assert(mkdtemp(dir) && "mkdtemp failed");        // create temp directory

    char plain[512], enc[512], dec[512];
    snprintf(plain, sizeof plain, "%s/plain.txt", dir); // build plaintext path
    snprintf(enc,   sizeof enc,   "%s/plain.enc",  dir); // expected encrypted path
    snprintf(dec,   sizeof dec,   "%s/plain.dec",  dir); // expected decrypted path

    write_file_simple(plain, "hello");               // seed plaintext file

    // 1) Encrypt -> creates .enc, deletes plaintext
    char pw1[] = "testpw";                           // writable buffer (gets zeroed by callee)
    assert(encrypt_inplace(plain, pw1, NULL) == 0);  // encrypt file in place (streaming)
    assert(access(plain, F_OK) != 0);                // plaintext removed
    assert(access(enc,   F_OK) == 0);                // .enc exists

    // 2) Decrypt -> creates .dec, deletes .enc
    char pw2[] = "testpw";                           // fresh writable buffer
    assert(decrypt_inplace(enc, pw2, ".dec") == 0);  // decrypt file in place with .dec suffix
    assert(access(enc, F_OK) != 0);                  // .enc removed
    assert(access(dec, F_OK) == 0);                  // .dec exists

    // 3) Check contents
    FILE *f = fopen(dec,"rb"); assert(f);            // open decrypted file
    char buf[16] = {0};                              // small buffer for readback
    fread(buf,1,sizeof buf,f); fclose(f);            // read and close
    assert(strcmp(buf,"hello") == 0);                // verify roundtrip content

    return 0;                                        // success
}


