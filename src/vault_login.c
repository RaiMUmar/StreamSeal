#include "../include/header.h"

/* Initialize New User */
static int init_user(){
    char pwd[1024]; char hashed[crypto_pwhash_STRBYTES]; const char *path = "user.pass"; // Initialize Variables

    if (prompt_password("Create Password: ", pwd, sizeof(pwd), 1) != 1) return -1;

    if (crypto_pwhash_str(hashed, pwd, strlen(pwd), crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0){ // Create the Hash
        printf("Could Not Create Hash!\n");
        sodium_memzero(pwd, sizeof(pwd));
        return -1;
    }

    sodium_memzero(pwd, sizeof(pwd));

    if (write_file(path, hashed, strlen(hashed)) != 1){ // Write the Hash
        sodium_memzero(hashed, sizeof(hashed));
        return -1;
    }


    printf("User Created\n");
    return 1;
}

/* Login User */
int login_user(){
    unsigned char *filebuf = NULL; const char *path = "user.pass"; size_t filelen = 0; char pwd[1024]; // Initialize Variables

    if (read_file(path, &filebuf, &filelen) != 1){ // Read Password File
        printf("No password file found!\n");
        return -1;
    }

    filebuf = realloc(filebuf, filelen + 1);
    if (!filebuf){
        printf("Realloc Failed!\n");
        return -1;
    }
    filebuf[filelen] = '\0';

    if (prompt_password("Enter Password: ", pwd, sizeof (pwd), 0) != 1){
        sodium_free(filebuf);
        return -1;
    }

    int success = crypto_pwhash_str_verify(filebuf, pwd, strlen(pwd)); // Check if Password Matches

    sodium_memzero(pwd, sideof(pwd));
    sodium_free(filebuf);

    if (success == 0){
        printf("Login Success\n");
        return 1;
    } else {
        printf("Login Failed\n");
        return -1;
    }
}