#include "../include/header.h"

int main(int argc, char **argv){
    char pwd[1024];

    if (sodium_init() < 0){
        printf("Program Failed!");
        return -1;
    }

    if (argc < 2) { 
        usage(argv[0]); 
        return -1; 
    }

    const char *cmd = argv[1]; 

    if (strcmp(cmd, "init-user") == 0) {
        return init_user() == 0 ? 0 : 1;


    } else if (strcmp(cmd, "encrypt") == 0) {
        if (login_user(pwd) == 0){
            const char *in_path = NULL;

            if (argc < 3) { 
                printf("File not provided!\n");
                usage(argv[0]); 
                return -1; 
            }

            in_path = argv[2];

            printf("Encrypting...\n");
            return path_handler(encrypt_inplace, in_path, pwd, NULL) == 0 ? 0 : 2;
        } else {
            return -1;
        }

    } else if (strcmp(cmd, "decrypt") == 0) {
        if (login_user(pwd) == 0){
            const char *in_path = NULL, *suffix = NULL;

            if (argc < 3) { 
                printf("File not provided!\n");
                usage(argv[0]); 
                return -1; 
            }

            if (argc > 3) {
                suffix = argv[3];
            } else {
                suffix = ".dec";
            }

            in_path = argv[2];

            printf("Decrypting...\n");
            return path_handler(decrypt_inplace, in_path, pwd, suffix) == 0 ? 0 : 3;
        } else {
            return -1;
        }

    } else {
        usage(argv[0]);
        return -1;
    }

    return -1;
}
