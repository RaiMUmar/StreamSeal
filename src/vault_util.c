#include "../include/header.h"

/* ends_with: return non-zero if string `s` ends with `suffix`, else 0.
   Handles NULL inputs gracefully (returns 0). */
int ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return 0; // guard: null inputs cannot match
    size_t ls = strlen(s), lt = strlen(suffix); // compute lengths
    return (lt <= ls) && (strcmp(s + (ls - lt), suffix) == 0); // compare tail of s with suffix
}

/* base_name: return pointer to last path component in `path`
   (portion after the final '/' or '\\' on Windows). */
const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/'); // find last forward slash
#ifdef _WIN32
    const char *bslash = strrchr(path, '\\'); // find last backslash (Windows)
    if (!slash || (bslash && bslash > slash)) slash = bslash; // prefer whichever separator appears last
#endif
    return slash ? slash + 1 : path; // if a separator exists, move past it; otherwise return whole path
}

