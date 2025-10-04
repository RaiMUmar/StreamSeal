#include "../include/header.h"

/* encrypt_inplace: encrypt `in_path` to a sibling file with ".enc" suffix using
   streaming; optionally deletes the source if g_delete_on_success is set.
   `garbage` is unused. Returns 0 on success, -1 on error. */
int encrypt_inplace(const char *in_path, char *pwd, const char *garbage) {
    if (!in_path || !pwd) return -1; // guard: validate pointers

    char out_path[4096];
    if (build_path(in_path, ".enc", out_path, sizeof out_path) != 0) return -1; // derive output path with .enc

    // If output would equal input, recompute to avoid path collision.
    if (strcmp(out_path, in_path) == 0) {
        if (build_path(in_path, ".enc", out_path, sizeof out_path) != 0) return -1; // rebuild output path
    }

    int rc = encrypt_file_stream(in_path, out_path, pwd); // stream-encrypt file into out_path
    // On success, optionally delete the original file if user opted in.
    if (rc == 0) {
        if (g_delete_on_success) {  // opt-in deletion of source on success
            if (safe_delete(in_path) != 0) // try to unlink original
                fprintf(stderr, "Warning: could not delete original file: %s\n", in_path); // warn if deletion failed
        }
        return 0; // success
    }
    return -1; // failure
}

