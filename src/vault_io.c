#include "../include/header.h"

/* Read Any File In Binary Form and Store Its Data Into buff Variable */
int read_file (const char*path, unsigned char **buff, size_t *len){
    *buff = NULL; *len = 0;

    FILE *fptr = fopen(path, "rb"); // Open File in Binary Read Mode
    if (fptr == NULL){ // Error Check
        printf("Error Opening File!\n");
        return -1;
    }

    if (fseek(fptr, 0, SEEK_END) != 0){ // Move Pointer to End of File
        printf("Fseek Error!\n");
        fclose(fptr);
        return -1;
    }

    long fileSize = ftell(fptr); // Set fileSize to the Length of the File
    if (fileSize < 0){ // Error Check
        printf("File Does Not Exist\n");
        fclose(fptr);
        return -1;
    }

    if (fseek(fptr, 0, SEEK_SET) != 0){ // Set Pointer to Start of File
        printf("Fseek Error!\n"); // Error Check
        fclose(fptr);
        return -1;
    }

    if (fileSize > 0){
        *buff = sodium_malloc((size_t)fileSize); // Malloc Enough Space To Read Entire File
        if (!*buff){ // Check for Malloc Error
            printf("Could Not Malloc!\n");
            fclose(fptr);
            return -1;
        }
    } else {
        *buff = sodium_malloc(1); // Malloc Enough Space To Read Entire File
        if (!*buff){ // Check for Malloc Error
            printf("Could Not Malloc!\n");
            fclose(fptr);
            return -1;
        }
    }
    

    size_t numRead = fread(*buff, 1, (size_t)fileSize, fptr); // Read File 
    if (numRead != (size_t)fileSize){
        printf("File Not Read Properly!\n");
        fclose(fptr);
        sodium_free(*buff);
        return -1;
    }

    fclose(fptr);
    *len = numRead;
    return 1; // Success
}


/* Write buffer Into A Binary File */
int write_file(const char *path, const unsigned char *buf, size_t len){

    FILE* fptr = fopen(path, "wb"); // Open Binary File To Write
    if (fptr == NULL){
        printf("Could Not Open File!\n");
        fclose(fptr);
        return -1;
    }

    size_t numWritten = fwrite(buf, 1, len, fptr); // Write Entire File

    if (numWritten != len){ // Error Check To See If Entire Fire Was Written
        printf("Could Not Write To File!\n");
        fclose(fptr);
        return -1;
    }

    fclose(fptr);
    return 1; // Success
}


int prompt_password(const char *label, char *out, size_t outsz, int confirm){
    char tmp[1024]; // Temp Buffer for Confirmation

    if (!out || outsz == 0) return -1;

    // Get Password And Store in out
    printf("%s", label);
    if (!fgets(out, outsz, stdin)) return -1;
    out[strcspn(out, "\r\n")] = '\0';  // Strip Newline if Present

    // Confirm Password
    if (confirm){
        printf("Confirm Password: ");
        if (!fgets(tmp, sizeof(tmp), stdin)) return -1;
        tmp[strcspn(tmp, "\r\n")] = '\0'; // Strip Newline if Present

            // Check if Passwords Match
        if (strcmp(out, tmp) != 0) {
            printf("Passwords do not match!\n");
            sodium_memzero(out, outsz);
            sodium_memzero(tmp, sizeof(tmp));
            return -1;
        }
    }
    
    sodium_memzero(tmp, sizeof(tmp));
    return 1; // Success
}
