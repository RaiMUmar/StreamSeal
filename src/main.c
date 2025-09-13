#include "../include/header.h"

int main(int argc, char **argv){

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
        return login_user() == 1 ? 1 : 3;


    } else if (strcmp(cmd, "encrypt") == 0) {
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
        //return encrypt_file(in_path, out_path) == 1 ? 1 : 4;
        return path_handler(in_path, out_path);

    } else if (strcmp(cmd, "decrypt") == 0) {
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
        return decrypt_file(in_path, out_path) == 1 ? 1 : 5;


    } else {
        usage(argv[0]);
        return 1;
    }

    return 1;
}
