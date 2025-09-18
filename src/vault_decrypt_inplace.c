#include "../include/header.h"

int decrypt_inplace(const char *in_path, char *pwd, const char *wanted_ext) {
    if (!in_path || !pwd) return -1;

    const char *ext = (wanted_ext && *wanted_ext) ? wanted_ext : ".dec";

    char out_path[4096];
    if (build_path(in_path, ext, out_path, sizeof out_path) != 0) {
        return -1;
    }

    /* Guard against the (unlikely) case that out_path == in_path */
    if (strcmp(out_path, in_path) == 0) {
        /* Append ".out" to ensure a distinct path. */
        size_t len = strlen(out_path);
        if (len + 4 + 1 >= sizeof out_path) return -1;
        strcat(out_path, ".out");
    }

    int rc = decrypt_file(in_path, out_path, pwd);
    if (rc == 0) {
        if (safe_delete(in_path) != 0) {
            fprintf(stderr, "Warning: could not delete original file: %s\n", in_path);
        }
        return 0;
    }
    return -1;
}
