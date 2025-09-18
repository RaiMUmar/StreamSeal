#include "../include/header.h"

/* Initialize New User */
int init_user(void){
    const char *path = "user.pass"; // Initialize Path Variable
    if (user_created(path)){
        printf("User has already been initialized!\n");
        return 0;
    }

    char pwd[1024]; char hashed[crypto_pwhash_STRBYTES]; 

    if (prompt_password("Create Password: ", pwd, sizeof(pwd), 1) != 0) return -1;

    if (crypto_pwhash_str(hashed, pwd, strlen(pwd), crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0){ // Create the Hash
        printf("Could Not Create Hash!\n");
        sodium_memzero(pwd, sizeof(pwd));
        return -1;
    }

    sodium_memzero(pwd, sizeof(pwd));

    if (write_file(path, (const unsigned char *)hashed, strlen(hashed)) != 0){ // Write the Hash
        sodium_memzero(hashed, sizeof(hashed));
        return -1;
    }


    printf("User Created\n");
    return 0;
}

/* Login User */
int login_user(char *pwd){
    unsigned char *filebuf = NULL; const char *path = "user.pass"; size_t filelen = 0; // Initialize Variables

    if (read_file(path, &filebuf, &filelen) != 0){ // Read Password File
        printf("User not initialized yet!\n");
        return -1;
    }

    unsigned char *filebuf2 = sodium_malloc(filelen+1);
    if (!filebuf2){
        printf("Malloc Failed!\n");
        sodium_free(filebuf);
        return -1;
    }
    memcpy(filebuf2, filebuf, filelen);
    filebuf2[filelen] = '\0';
    sodium_free(filebuf);
    filebuf = filebuf2;

    if (prompt_password("Enter Password: ", pwd, sizeof(pwd), 0) != 0){
        sodium_free(filebuf);
        return -1;
    }

    int success = crypto_pwhash_str_verify((const char *)filebuf, pwd, strlen(pwd)); // Check if Password Matches

    sodium_free(filebuf);

    if (success == 0){
        printf("Login Success\n");
        return 0; // Success
    } else {
        printf("Login Failed\n");
        return -1;
    }
}

int user_created(const char *path){
    FILE *fptr = fopen(path, "rb");
    if (fptr == NULL){
        return -1;
    }
    fclose(fptr);
    return 0;
}
