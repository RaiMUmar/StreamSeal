#include "../include/header.h"

int main(int argc, char **argv){
    char pwd[1024];

    if (sodium_init() < 0){
        printf("Program Failed!");
        return 0;
    }

    if (argc < 2) { 
        usage(argv[0]); 
        return 1; 
    }

    const char *cmd = argv[1]; 

    if (strcmp(cmd, "init-user") == 0) {
        return init_user() == 1 ? 1 : 2;


    } else if (strcmp(cmd, "login") == 0) {
        return login_user(pwd) == 1 ? 1 : 3;


    } else if (strcmp(cmd, "encrypt") == 0) {
        if (login_user(pwd)){
            const char *in_path = NULL;

            if (argc < 3) { 
                printf("File not provided!\n");
                usage(argv[0]); 
                return 1; 
            }

            in_path = argv[2];

            printf("Encrypting...\n");
            return path_handler(encrypt_inplace, in_path, pwd);
        }

    } else if (strcmp(cmd, "decrypt") == 0) {
        if (login_user(pwd)){
            const char *in_path = NULL;

            if (argc < 3) { 
                printf("File not provided!\n");
                usage(argv[0]); 
                return 1; 
            }

            in_path = argv[2];

            printf("Decrypting...\n");
            return path_handler(decrypt_inplace, in_path, pwd);
        }

    } else {
        usage(argv[0]);
        return 1;
    }

    return 1;
}
