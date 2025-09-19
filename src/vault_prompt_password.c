#include "../include/header.h"

static int read_hidden(const char *prompt, char *buf, size_t bufsz) {
    FILE *tty = fopen("/dev/tty", "r+");
    if (!tty) tty = stdin;                   // fallback if no controlling TTY
    int fd = fileno(tty);

    struct termios oldt, noecho;
    if (tcgetattr(fd, &oldt) != 0) { if (tty != stdin) fclose(tty); return -1; }
    noecho = oldt;
    noecho.c_lflag &= ~(ECHO);
    if (tcsetattr(fd, TCSAFLUSH, &noecho) != 0) { if (tty != stdin) fclose(tty); return -1; }

    fprintf(tty, "%s", prompt); fflush(tty);

    if (!fgets(buf, bufsz, tty)) {
        tcsetattr(fd, TCSANOW, &oldt);
        if (tty != stdin) fclose(tty);
        return -1;
    }

    // Strip newline
    buf[strcspn(buf, "\r\n")] = '\0';

    // Restore echo and print a newline so the cursor moves down
    tcsetattr(fd, TCSANOW, &oldt);
    fputc('\n', tty); fflush(tty);
    if (tty != stdin) fclose(tty);
    return 0;
}

int prompt_password(const char *label, char *out, size_t outsz, int confirm) {
    char tmp[1024];

    if (!out || outsz == 0) return -1;

    if (read_hidden(label, out, outsz) != 0) return -1;

    if (confirm) {
        if (read_hidden("Confirm Password: ", tmp, sizeof tmp) != 0) {
            sodium_memzero(out, outsz);
            return -1;
        }
        if (strcmp(out, tmp) != 0) {
            printf("Passwords do not match!\n");
            sodium_memzero(tmp, sizeof tmp);
            sodium_memzero(out, outsz);
            return -1;
        }
        sodium_memzero(tmp, sizeof tmp);
    }
    return 0;
}
