#include "../include/header.h"

static int init_user(){
    char pwd[1024]; char hashed[crypto_pwhash_STRBYTES]; const char *path = "user.pass"; // Initialize Variables

    if (prompt_password("Create Password: ", pwd, sizeof(pwd), 1) != 1) return -1;

    if (crypto_pwhash_str(hashed, pwd, strlen(pwd), crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0){ // Create the Hash
        printf("Could Not Create Hash!\n");
        sodium_memzero(pwd, sizeof(pwd));
        return -1;
    }

    sodium_memzero(pwd, sizeof(pwd));

    if (write_file(path, hashed, strlen(hashed)) != 1){
        sodium_memzero(hashed, sizeof(hashed));
        return -1;
    }


    printf("User Created\n");
    return 1;
}