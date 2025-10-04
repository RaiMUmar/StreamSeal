#include "../include/header.h"

/* build_path: derive an output path from an input path by replacing the
   basename's extension with the given suffix. Writes into out_path (size out_sz).
   Returns 0 on success, -1 on error (invalid args/overflow). */
int build_path(const char *in_path, const char *suffix, char *out_path,size_t out_sz) {
    if (!in_path || !suffix || !out_path || out_sz == 0) return -1; // validate inputs

    const char *last_slash = strrchr(in_path, '/'); // find last forward slash (POSIX)
#ifdef _WIN32
    const char *last_bslash = strrchr(in_path, '\\'); // find last backslash (Windows)
    if (!last_slash || (last_bslash && last_bslash > last_slash)) {
        last_slash = last_bslash; // prefer backslash if later in string
    }
#endif

    size_t dir_len = 0;
    // If a directory separator exists, copy the directory prefix into out_path.
    if (last_slash) {
        dir_len = (size_t)(last_slash - in_path) + 1; // include the slash
        if (dir_len >= out_sz) return -1; // avoid overflow of out_path
        memcpy(out_path, in_path, dir_len); // copy directory component
        out_path[dir_len] = '\0'; // terminate the directory prefix
    } else {
        out_path[0] = '\0'; // no directory; start with empty prefix
    }

    const char *base = last_slash ? (last_slash + 1) : in_path; // point to basename

    /* Find the last dot in the basename that is not the first character (to avoid ".bashrc" edge). */
    const char *last_dot = strrchr(base, '.'); // find extension separator
    size_t base_core_len = (last_dot && last_dot > base) ? (size_t)(last_dot - base) : strlen(base); // compute basename without extension

    /* Compose: dir + base_core + suffix */
    // Ensure enough space for directory + core name + suffix + NUL.
    if (dir_len + base_core_len + strlen(suffix) + 1 > out_sz) return -1; // capacity check
    memcpy(out_path + dir_len, base, base_core_len); // copy core basename (without extension)
    strcpy(out_path + dir_len + base_core_len, suffix); // append requested suffix
    return 0; // success
}

