#include "../include/header.h"

/* init_user: create a new credential file "user.pass" with an Argon2id-hashed
   password, written atomically with 0600 permissions. Returns 0 on success, -1 on error. */
int init_user(void){
    const char *path = "user.pass"; // target credential file
    // Guard: refuse to re-initialize if the credential file already exists.
    if (user_created(path) != 0){ // check for existing user.pass
        printf("User has already been initialized!\n");
        return 0;
    }

    char pwd[1024];  // password input buffer
    char hashed[crypto_pwhash_STRBYTES]; // storage for Argon2id hash string

    // Prompt for password (with confirmation); fail on input error.
    if (prompt_password("Create Password: ", pwd, sizeof(pwd), 1) != 0) return -1; // read & confirm password

    // Derive password hash with Argon2id; bail on failure (e.g., OOM).
    if (crypto_pwhash_str(hashed, pwd, strlen(pwd),
                          crypto_pwhash_OPSLIMIT_MODERATE,
                          crypto_pwhash_MEMLIMIT_MODERATE) != 0){ // hash password with Argon2id
        printf("Could Not Create Hash!\n");
        sodium_memzero(pwd, sizeof(pwd)); // scrub plaintext password
        return -1;
    }
    sodium_memzero(pwd, sizeof(pwd)); // scrub plaintext password after hashing

    // Atomically write the hash to disk with strict perms.
    if (write_file_atomic_0600(path, (const unsigned char *)hashed, strlen(hashed)) != 0){ // atomic 0600 write
        sodium_memzero(hashed, sizeof(hashed)); // scrub hash buffer
        perror("write_file_atomic_0600"); // surface errno reason
        return -1;
    }
    sodium_memzero(hashed, sizeof(hashed)); // scrub hash buffer

    printf("User Created\n"); // success feedback
    return 0;
}

/* login_user: verify a password against the stored Argon2id hash in "user.pass".
   Returns 0 on success, -1 on failure (not initialized, bad input, or mismatch). */
int login_user(char *pwd){
    unsigned char *filebuf = NULL; const char *path = "user.pass"; size_t filelen = 0; // Initialize Variables

    // Preflight: if file is present but perms too wide, tighten to 0600.
    struct stat st;
    if (stat(path, &st) == 0) { // fetch file mode
        // If group/other have any perms, warn and restrict to 0600.
        if ((st.st_mode & (S_IRWXG | S_IRWXO)) != 0) {
            fprintf(stderr, "Warning: %s permissions are too wide; tightening to 0600.\n", path);
            (void)chmod(path, S_IRUSR | S_IWUSR); // best-effort fix
        }
    }

    // Load the stored hash string into memory.
    if (read_file(path, &filebuf, &filelen) != 0){ // read password file
        printf("User not initialized yet!\n");
        return -1;
    }

    // Allocate a NUL-terminated copy (crypto_pwhash_str_verify expects a C string).
    unsigned char *filebuf2 = sodium_malloc(filelen+1); // +1 for NUL
    if (!filebuf2){
        printf("Malloc Failed!\n");
        sodium_free(filebuf); // free original buffer
        return -1;
    }
    memcpy(filebuf2, filebuf, filelen); // copy hash bytes
    filebuf2[filelen] = '\0'; // ensure NUL termination
    sodium_free(filebuf); // discard original buffer
    filebuf = filebuf2; // use the NUL-terminated buffer

    // Prompt for password (no confirmation).
    if (prompt_password("Enter Password: ", pwd, sizeof(pwd), 0) != 0){ // read password
        sodium_free(filebuf); // free hash buffer
        return -1;
    }

    // Verify password against the stored hash.
    int success = crypto_pwhash_str_verify((const char *)filebuf, pwd, strlen(pwd)); // verify Argon2id hash

    sodium_free(filebuf); // free hash buffer

    // Branch: success is 0 when verification passes.
    if (success == 0){
        printf("Login Success\n");
        return 0; // Success
    } else {
        printf("Login Failed\n");
        return -1;
    }
}

/* user_created: probe for existence of `path` by attempting to open it for reading.
   Returns -1 if present, 0 if absent. */
int user_created(const char *path){
    FILE *fptr = fopen(path, "rb"); // try opening file to check existence
    if (fptr == NULL){
        return 0; // not present
    }
    fclose(fptr); // close handle
    return -1; // present
}

