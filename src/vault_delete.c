#include "../include/header.h"

int safe_delete(const char *path) {
    return remove(path) == 0 ? 0 : -1;
}
