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
            printf("Encrypting...\n");
            const char *in_path = NULL, *out_path = NULL;

            for (int i = 2; i + 1 < argc; i++) {
                if (strcmp(argv[i], "-in") == 0) 
                    in_path = argv[++i];
                else if (strcmp(argv[i], "-out") == 0) 
                    out_path = argv[++i];
            }

            if (!in_path || !out_path) { 
                usage(argv[0]); 
                return 1; 
            }
            return path_handler(encrypt_file, in_path, out_path, pwd);
        }

    } else if (strcmp(cmd, "decrypt") == 0) {
        if (login_user(pwd)){
            printf("Decrypting...\n");
            const char *in_path = NULL, *out_path = NULL;

            for (int i = 2; i + 1 < argc; i++) {
                if (strcmp(argv[i], "-in") == 0) 
                    in_path = argv[++i];
                else if (strcmp(argv[i], "-out") == 0) 
                    out_path = argv[++i];
            }
            if (!in_path || !out_path) { 
                usage(argv[0]); 
                return 1; 
            }
            return path_handler(decrypt_file, in_path, out_path, pwd);
        }

    } else {
        usage(argv[0]);
        return 1;
    }

    return 1;
}
