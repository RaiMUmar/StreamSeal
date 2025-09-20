#include <assert.h>
#include "../include/header.h"

static void check(const char *in, const char *suf, const char *want) {
    char out[4096];
    assert(build_path(in, suf, out, sizeof out) == 0);
    assert(strcmp(out, want) == 0);
}

int main(void) {
    check("dir/file.txt", ".enc", "dir/file.enc");
    check("file", ".enc", "file.enc");
    check(".bashrc", ".enc", ".bashrc.enc");
    check("dir/a.b.c", ".dec", "dir/a.b.dec");
    return 0;
}
