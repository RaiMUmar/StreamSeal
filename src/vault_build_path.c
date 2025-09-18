#include "../include/header.h"

int build_path(const char *in_path,
                                                const char *suffix,
                                                char *out_path,
                                                size_t out_sz)
{
    if (!in_path || !suffix || !out_path || out_sz == 0) return -1;

    const char *last_slash = strrchr(in_path, '/');
#ifdef _WIN32
    const char *last_bslash = strrchr(in_path, '\\');
    if (!last_slash || (last_bslash && last_bslash > last_slash)) {
        last_slash = last_bslash;
    }
#endif

    size_t dir_len = 0;
    if (last_slash) {
        dir_len = (size_t)(last_slash - in_path) + 1; // include the slash
        if (dir_len >= out_sz) return -1;
        memcpy(out_path, in_path, dir_len);
        out_path[dir_len] = '\0';
    } else {
        out_path[0] = '\0';
    }

    const char *base = last_slash ? (last_slash + 1) : in_path;

    /* Find the last dot in the basename that is not the first character (to avoid ".bashrc" edge). */
    const char *last_dot = strrchr(base, '.');
    size_t base_core_len = (last_dot && last_dot > base) ? (size_t)(last_dot - base) : strlen(base);

    /* Compose: dir + base_core + suffix */
    if (dir_len + base_core_len + strlen(suffix) + 1 > out_sz) return -1;
    memcpy(out_path + dir_len, base, base_core_len);
    strcpy(out_path + dir_len + base_core_len, suffix);
    return 0;
}
