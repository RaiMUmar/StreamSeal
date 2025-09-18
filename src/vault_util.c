#include "../include/header.h"

int ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return 0;
    size_t ls = strlen(s), lt = strlen(suffix);
    return (lt <= ls) && (strcmp(s + (ls - lt), suffix) == 0);
}

const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/');
#ifdef _WIN32
    const char *bslash = strrchr(path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif
    return slash ? slash + 1 : path;
}
