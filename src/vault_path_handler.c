#include "../include/header.h"

int path_handler(const char *path, const char *notNeeded){
    struct stat path_stat;

    if (stat(path, &path_stat) != 0){
        printf("Stat Failed!\n");
        return -1;
    }

    if (S_ISREG(path_stat.st_mode)){    // It is a file
        char new_path[1024];
        snprintf(new_path, sizeof(new_path), "%s_encrypted", path);
        encrypt_file (path, new_path);
        return 1; 

    } else if (S_ISDIR(path_stat.st_mode)) {    // It is a folder
        DIR *dir = opendir(path); // Open Directory
        if (!dir){
            printf("Failed to open folder!\n");
            return -1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != 0){
            // Skip "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue; 

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            path_handler(full_path, "Hello");
        }
    closedir(dir);
    }

    return 1;
}
