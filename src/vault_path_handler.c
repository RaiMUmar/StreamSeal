#include "../include/header.h"

int path_handler(encrypt_func f, const char *path, char *pwd, const char *suffix){
    struct stat path_stat;

    if (stat(path, &path_stat) != 0){
        printf("Stat Failed!\n");
        return -1;
    }

    if (S_ISREG(path_stat.st_mode)){    // It is a file
        if (f(path, pwd, suffix) != 0){
            return -1;
        }
        return 0; 

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
            
            if (path_handler(f, full_path, pwd, suffix) != 0){
                closedir(dir);
                return -1;
            }
        }
    closedir(dir);
    }

    return 0;
}
