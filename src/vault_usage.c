#include "../include/header.h"

/* usage: print CLI help/usage summary, options, and notes to stderr.
   `prog` should be argv[0] so the help shows the correct executable name. */
void usage(const char *prog) {
    fprintf(stderr,  // print a multi-line formatted usage message
        "Usage:\n"
        "  %s init-user\n"
        "  %s encrypt <path> [--rm]\n"
        "  %s decrypt <path> [suffix] [--rm]\n"
        "\n"
        "Options:\n"
        "  --rm, --delete   Remove source on success (opt-in)\n"
        "\n"
        "Notes:\n"
        "  • Symlinks and special files (devices, fifos, sockets) are skipped.\n",
        prog, prog, prog); // substitute executable name in all lines
}

