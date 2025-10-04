#include "../include/header.h"

/* read_hidden: read a line from the controlling TTY with echo disabled.
   Prompts with `prompt`, stores into `buf` (size `bufsz`), strips newline.
   Returns 0 on success, -1 on error. */
static int read_hidden(const char *prompt, char *buf, size_t bufsz) {
    FILE *tty = fopen("/dev/tty", "r+"); // open controlling TTY for read/write
    if (!tty) tty = stdin;                   // fallback if no controlling TTY
    int fd = fileno(tty); // get file descriptor for termios ops

    struct termios oldt, noecho;
    // Disable echo: save current settings, then turn off ECHO.
    if (tcgetattr(fd, &oldt) != 0) { if (tty != stdin) fclose(tty); return -1; } // get current termios
    noecho = oldt; // copy original settings
    noecho.c_lflag &= ~(ECHO); // clear echo flag
    if (tcsetattr(fd, TCSAFLUSH, &noecho) != 0) { if (tty != stdin) fclose(tty); return -1; } // apply no-echo mode

    fprintf(tty, "%s", prompt); fflush(tty); // print prompt and flush to TTY

    // Read one line; on failure, restore termios and clean up.
    if (!fgets(buf, bufsz, tty)) {
        tcsetattr(fd, TCSANOW, &oldt); // restore original termios
        if (tty != stdin) fclose(tty); // close TTY if we opened it
        return -1;
    }

    // Strip newline
    buf[strcspn(buf, "\r\n")] = '\0'; // remove trailing CR/LF

    // Restore echo and print a newline so the cursor moves down
    tcsetattr(fd, TCSANOW, &oldt); // restore original termios
    fputc('\n', tty); fflush(tty); // move to next line visually
    if (tty != stdin) fclose(tty); // close TTY if we opened it
    return 0; // success
}

/* prompt_password: prompt for a password with echo disabled.
   If `confirm` is non-zero, prompts twice and verifies they match.
   Writes into `out` (size `outsz`). Returns 0 on success, -1 on error. */
int prompt_password(const char *label, char *out, size_t outsz, int confirm) {
    char tmp[1024]; // temp buffer for confirmation

    // Validate output buffer params.
    if (!out || outsz == 0) return -1;

    // First prompt for password (hidden input).
    if (read_hidden(label, out, outsz) != 0) return -1; // read first entry securely

    // If confirmation required, prompt again and compare.
    if (confirm) {
        if (read_hidden("Confirm Password: ", tmp, sizeof tmp) != 0) { // read confirmation securely
            sodium_memzero(out, outsz); // scrub first entry on failure
            return -1;
        }
        // Mismatch check between first and second entries.
        if (strcmp(out, tmp) != 0) {
            printf("Passwords do not match!\n");
            sodium_memzero(tmp, sizeof tmp); // scrub temp
            sodium_memzero(out, outsz); // scrub output
            return -1;
        }
        sodium_memzero(tmp, sizeof tmp); // scrub temp on success
    }
    return 0; // success
}

