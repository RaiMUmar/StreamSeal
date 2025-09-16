#include "../include/header.h"

/* Shows Available Commands to User */
void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        " %s init-user\n"
        " %s login\n"
        " %s encrypt <plain>\n"
        " %s decrypt <cipher>\n",
        prog, prog, prog, prog);
}
