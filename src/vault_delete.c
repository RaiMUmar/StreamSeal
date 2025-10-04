#include "../include/header.h"

/* safe_delete: unlink the file at `path`.
   Returns 0 on success, -1 on failure. */
int safe_delete(const char *path) {
    return remove(path) == 0 ? 0 : -1; // call C stdlib remove(); map result to 0/-1
}

