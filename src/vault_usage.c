#include "../include/header.h"

static void usage(const char *prog) {
    printf(stderr,
        "Usage:\n"
        " %s init-user\n"
        " %s login\n"
        " %s encrypt -in <plain> -out <cipher>\n"
        " %s decrypt -in <cipher> -out <plain>\n",
        prog, prog, prog, prog);
}