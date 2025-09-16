#include "../include/header.h"

/* Shows Available Commands to User */
void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        " %s init-user\n"
        " %s encrypt <plain>\n"
        " %s decrypt <cipher> <suffix>\n",
        prog, prog, prog);
}
