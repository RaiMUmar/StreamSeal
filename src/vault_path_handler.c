#include "../include/header.h"

/* path_handler: dispatches an encrypt/decrypt function `f` over a path.
   - For regular files: applies `f` (skips user.pass, already .enc/.dec).
   - For directories: recurses into entries (skips . and ..).
   - Skips symlinks/devices/FIFOs/sockets by using lstat and only handling
     S_ISREG and S_ISDIR.
   Returns 0 on success/skip, -1 on error or child error. */
int path_handler(encrypt_func f, const char *path, char *pwd, const char *suffix){
    struct stat st;

    // Stat the path without following symlinks (POSIX lstat; Windows stat fallback).
    /* use lstat so symlinks are not followed; on Windows fall back to stat */
#if defined(_WIN32)
    if (stat(path, &st) != 0) { perror("stat"); return -1; } // fetch metadata (Windows)
#else
    if (lstat(path, &st) != 0) { perror("lstat"); return -1; } // fetch metadata without following symlinks (POSIX)
#endif

    // Branch: handle regular files.
    if (S_ISREG(st.st_mode)) {               /* regular file only */
        const char *name = base_name(path); // get basename from path

        // Never operate on credential file.
        if (strcmp(name, "user.pass") == 0) return 0;  /* never touch creds */

        // Skip files that are already in the target state.
        if (f == encrypt_inplace && ends_with(name, ".enc")) return 0; // skip already-encrypted files
        if (f == decrypt_inplace && ends_with(name, ".dec")) return 0; // skip .dec files during decrypt

        return f(path, pwd, suffix); // apply operation to this regular file

    // Branch: handle directories (recurse).
    } else if (S_ISDIR(st.st_mode)) {        /* recurse directories */
        DIR *dir = opendir(path); // open directory stream
        if (!dir){ perror("opendir"); return -1; } // fail if cannot open

        int rc = 0;
        struct dirent *entry;
        // Iterate directory entries (skip "." and "..").
        while ((entry = readdir(dir)) != 0){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue; // skip self/parent entries

            char full_path[1024];
            snprintf(full_path, sizeof full_path, "%s/%s", path, entry->d_name); // build child path

            int child = path_handler(f, full_path, pwd, suffix); // recurse into child path
            if (child != 0) { rc = child; break; } // propagate first non-zero (error) and stop
        }
        closedir(dir); // close directory stream
        return rc; // return aggregate result from recursion
    }

    // Not a regular file or directory (symlink, device, fifo, socket) → skip.
    /* Not a regular file or directory: symlink, device, fifo, socket → skip */
    return 0; // treat as no-op
}

