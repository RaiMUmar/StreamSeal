#include "../include/header.h"

/* main: fuzz-smoke test harness for decrypt_file().
   Randomly generates inputs, writes them to temp files, and ensures decryption
   does not crash; silences stdout/stderr during the run. */
int main(int argc, char **argv) {
    int runs = 1000; // default number of iterations
    // Parse optional '-runs=N' from argv to override iteration count.
    for (int i = 1; i < argc; ++i) (void)sscanf(argv[i], "-runs=%d", &runs); // parse runs override

    // Initialize libsodium; return non-zero on failure so CI flags it.
    if (sodium_init() < 0) return 1; // init crypto library

    /* ---- silence stdout/stderr while fuzzing ---- */
    FILE *devnull = fopen("/dev/null", "w"); // open bit bucket for output redirection
    int saved_out = dup(STDOUT_FILENO); // save current stdout fd
    int saved_err = dup(STDERR_FILENO); // save current stderr fd
    // If /dev/null opened, redirect stdout/stderr to it.
    if (devnull) {
        dup2(fileno(devnull), STDOUT_FILENO); // redirect stdout
        dup2(fileno(devnull), STDERR_FILENO); // redirect stderr
    }

    // Fuzzing loop: run `runs` iterations of random-input decryption.
    for (int it = 0; it < runs; ++it) {
        uint32_t r = 0;
        randombytes_buf(&r, sizeof(r)); // fill r with random bytes
        size_t sz = (size_t)(r % (64 * 1024 + 1)); // choose size in [0, 64KiB]

        unsigned char *buf = (unsigned char*)malloc(sz ? sz : 1); // allocate buffer (>=1)
        if (!buf) break; // stop if OOM
        if (sz) randombytes_buf(buf, sz); // randomize buffer contents

        char in[]  = "/tmp/fz-XXXXXX"; // mkstemp template for input file
        int fd_in  = mkstemp(in); // create unique temp input file
        if (fd_in < 0) { free(buf); break; } // bail on failure
        (void)write(fd_in, buf, sz);  // write fuzz data to input file
        close(fd_in); // close input fd

        char out[] = "/tmp/fz-out-XXXXXX"; // mkstemp template for output file
        int fd_out = mkstemp(out); close(fd_out); unlink(out); // reserve a path, then remove it

        char pwd[] = "pw"; // dummy password (not secret)
        (void)decrypt_file(in, out, pwd);  // should never crash // exercise decrypt on fuzz input

        unlink(in); // remove input file
        free(buf); // free fuzz buffer
    }

    /* ---- restore stdout/stderr ---- */
    fflush(stdout); fflush(stderr); // flush any buffered output
    if (saved_out >= 0) { dup2(saved_out, STDOUT_FILENO); close(saved_out); } // restore stdout
    if (saved_err >= 0) { dup2(saved_err, STDERR_FILENO); close(saved_err); } // restore stderr
    if (devnull) fclose(devnull); // close /dev/null stream if opened

    return 0; // success
}


