#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sodium.h>
#include "../include/header.h"

static void write_file_simple(const char *p, const char *s){
    FILE *f = fopen(p,"wb"); assert(f);
    fwrite(s,1,strlen(s),f);
    fclose(f);
}

int main(void){
    assert(sodium_init() >= 0);

    char dir[] = "/tmp/vault-XXXXXX";
    assert(mkdtemp(dir) && "mkdtemp failed");

    char plain[512], enc[512], dec[512];
    snprintf(plain, sizeof plain, "%s/plain.txt", dir);
    snprintf(enc,   sizeof enc,   "%s/plain.enc",  dir);
    snprintf(dec,   sizeof dec,   "%s/plain.dec",  dir);

    write_file_simple(plain, "hello");

    // 1) Encrypt -> creates .enc, deletes plaintext
    char pw1[] = "testpw";                 // writable buffer (gets zeroed)
    assert(encrypt_inplace(plain, pw1, NULL) == 0);
    assert(access(plain, F_OK) != 0);      // plaintext removed
    assert(access(enc,   F_OK) == 0);      // .enc exists

    // 2) Decrypt -> creates .dec, deletes .enc
    char pw2[] = "testpw";                 // fresh writable buffer
    assert(decrypt_inplace(enc, pw2, ".dec") == 0);
    assert(access(enc, F_OK) != 0);        // .enc removed
    assert(access(dec, F_OK) == 0);        // .dec exists

    // 3) Check contents
    FILE *f = fopen(dec,"rb"); assert(f);
    char buf[16] = {0};
    fread(buf,1,sizeof buf,f); fclose(f);
    assert(strcmp(buf,"hello") == 0);

    return 0;
}

