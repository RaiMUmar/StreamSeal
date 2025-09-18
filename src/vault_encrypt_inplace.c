#include "../include/header.h"

int encrypt_inplace(const char *in_path, char *pwd, const char *garbage) {
    if (!in_path || !pwd) return -1;

    char out_path[4096];
    if (build_path(in_path, ".enc", out_path, sizeof out_path) != 0) {
        return -1;
    }

    /* If out_path equals in_path, add .enc to avoid colliding with the source path. */
    if (strcmp(out_path, in_path) == 0) {
        if (build_path(in_path, ".enc", out_path, sizeof out_path) != 0) {
            return -1;
        }
    }

    int rc = encrypt_file(in_path, out_path, pwd);
    if (rc == 0) {
        /* Only delete the original if we successfully wrote the new file. */
        if (safe_delete(in_path) != 0) {
            /* If deletion fails, report but still consider the encryption a success. */
            fprintf(stderr, "Warning: could not delete original file: %s\n", in_path);
        }
        return 0;
    }
    /* On failure, do not delete anything. */
    return -1;
}
