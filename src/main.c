#include "../include/header.h"

/* main: entry point. Initializes libsodium, parses command and flags,
   prompts for password when needed, and dispatches to encrypt/decrypt/init. */
int main(int argc, char **argv){
    char pwd[1024]; // password buffer (writable; may be zeroed by callees)

    // Initialize libsodium; bail out if the crypto library can't start.
    if (sodium_init() < 0){
        printf("Program Failed!"); // report initialization failure
        return -1;
    }

    // Ensure a subcommand is provided (init-user/encrypt/decrypt).
    if (argc < 2) { 
        usage(argv[0]); // print usage/help
        return -1; 
    }

    const char *cmd = argv[1]; // first argument is the subcommand

    // Global flag scan: enable opt-in deletion if --rm/--delete is present.
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--rm") == 0 || strcmp(argv[i], "--delete") == 0) {
            g_delete_on_success = 1; // set global toggle for delete-on-success
        }
    }

    // Handle "init-user": create user.pass with hashed password.
    if (strcmp(cmd, "init-user") == 0) {
        return init_user() == 0 ? 0 : 1; // run initializer and map result to exit code

    // Handle "encrypt": require login, then encrypt file/dir.
    } else if (strcmp(cmd, "encrypt") == 0) {
        if (login_user(pwd) == 0){
            const char *in_path = NULL; // path to input (file or directory)

            // Require a path argument.
            if (argc < 3) { 
                printf("File not provided!\n"); // notify missing input
                usage(argv[0]); // show usage for correct invocation
                return -1; 
            }

            in_path = argv[2]; // capture input path argument

            printf("Encrypting...\n"); // user feedback
            return path_handler(encrypt_inplace, in_path, pwd, NULL) == 0 ? 0 : 2; // recurse/dispatch over path
        } else {
            return -1; // login failed
        }

    // Handle "decrypt": require login, then decrypt file/dir (optional suffix).
    } else if (strcmp(cmd, "decrypt") == 0) {
        if (login_user(pwd) == 0){
            const char *in_path = NULL, *suffix = NULL; // input path and output suffix

            // Require a path argument.
            if (argc < 3) { 
                printf("File not provided!\n"); // notify missing input
                usage(argv[0]); // show usage for correct invocation
                return -1; 
            }

            // Parse optional suffix argument; default to ".dec".
            if (argc > 3) {
                suffix = argv[3]; // caller-provided suffix for output
            } else {
                suffix = ".dec"; // default suffix
            }

            in_path = argv[2]; // capture input path argument

            printf("Decrypting...\n"); // user feedback
            return path_handler(decrypt_inplace, in_path, pwd, suffix) == 0 ? 0 : 3; // recurse/dispatch over path
        } else {
            return -1; // login failed
        }

    // Unknown subcommand: print usage and fail.
    } else {
        usage(argv[0]); // show valid commands
        return -1;
    }

    return -1; // unreachable in normal flows; defensive default
}

