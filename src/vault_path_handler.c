#include "../include/header.h"

int path_handler(encrypt_func f, const char *path, char *pwd, const char *suffix){
    struct stat path_stat;

    if (stat(path, &path_stat) != 0){
        perror("stat");
        return -1;
    }

    if (S_ISREG(path_stat.st_mode)) {
        const char *name = base_name(path);

        // Never touch user.pass
        if (strcmp(name, "user.pass") == 0) {
            return 0; // skip silently
        }

        // Skip files based on operation type
        if (f == encrypt_inplace && ends_with(name, ".enc")) {
            return 0; // skip already-encrypted
        }
        if (f == decrypt_inplace && ends_with(name, ".dec")) {
            return 0; // skip .dec files entirely
        }

        // Do the work
        return f(path, pwd, suffix);

    } else if (S_ISDIR(path_stat.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir){
            perror("opendir");
            return -1;
        }

        int rc = 0;
        struct dirent *entry;
        while ((entry = readdir(dir)) != 0){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            // You can optionally skip here too, but it’s only needed for files.
            int child = path_handler(f, full_path, pwd, suffix);
            if (child != 0) { rc = child; break; }
        }
        closedir(dir);
        return rc;
    }

    // Not file or dir: skip
    return 0;
}
