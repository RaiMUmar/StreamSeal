#include "../include/header.h"

/* check: helper to validate that build_path(in, suf) equals want.
   Uses asserts to enforce both success and exact string match. */
static void check(const char *in, const char *suf, const char *want) {
    char out[4096]; // destination buffer for computed path
    assert(build_path(in, suf, out, sizeof out) == 0); // ensure path built successfully
    assert(strcmp(out, want) == 0); // verify computed path matches expected
}

/* main: unit tests for build_path() covering common and edge cases. */
int main(void) {
    check("dir/file.txt", ".enc", "dir/file.enc"); // replace extension in a nested path
    check("file", ".enc", "file.enc");             // add suffix when no extension
    check(".bashrc", ".enc", ".bashrc.enc");       // handle dotfile without stripping leading dot
    check("dir/a.b.c", ".dec", "dir/a.b.dec");     // replace only the last extension
    return 0; // all assertions passed
}

