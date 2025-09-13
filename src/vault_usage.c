#include "../include/header.h"

/* Shows Available Commands to User */
void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        " %s init-user\n"
        " %s login\n"
        " %s encrypt -in <plain> -out <cipher>\n"
        " %s decrypt -in <cipher> -out <plain>\n",
        prog, prog, prog, prog);
}
