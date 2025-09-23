// tests/fuzz_smoke.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sodium.h>
#include "../include/header.h"

int main(int argc, char **argv) {
    int runs = 1000;
    for (int i = 1; i < argc; ++i) (void)sscanf(argv[i], "-runs=%d", &runs);

    if (sodium_init() < 0) return 1;

    /* ---- silence stdout/stderr while fuzzing ---- */
    FILE *devnull = fopen("/dev/null", "w");
    int saved_out = dup(STDOUT_FILENO);
    int saved_err = dup(STDERR_FILENO);
    if (devnull) {
        dup2(fileno(devnull), STDOUT_FILENO);
        dup2(fileno(devnull), STDERR_FILENO);
    }

    for (int it = 0; it < runs; ++it) {
        uint32_t r = 0;
        randombytes_buf(&r, sizeof(r));
        size_t sz = (size_t)(r % (64 * 1024 + 1));

        unsigned char *buf = (unsigned char*)malloc(sz ? sz : 1);
        if (!buf) break;
        if (sz) randombytes_buf(buf, sz);

        char in[]  = "/tmp/fz-XXXXXX";
        int fd_in  = mkstemp(in);
        if (fd_in < 0) { free(buf); break; }
        (void)write(fd_in, buf, sz);
        close(fd_in);

        char out[] = "/tmp/fz-out-XXXXXX";
        int fd_out = mkstemp(out); close(fd_out); unlink(out);

        char pwd[] = "pw";
        (void)decrypt_file(in, out, pwd);  // should never crash

        unlink(in);
        free(buf);
    }

    /* ---- restore stdout/stderr ---- */
    fflush(stdout); fflush(stderr);
    if (saved_out >= 0) { dup2(saved_out, STDOUT_FILENO); close(saved_out); }
    if (saved_err >= 0) { dup2(saved_err, STDERR_FILENO); close(saved_err); }
    if (devnull) fclose(devnull);

    return 0;
}

