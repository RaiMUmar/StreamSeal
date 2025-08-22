#include "../include/header.h"

/* Read Any File In Binary Form and Store Its Data Into buff Variable */
static int read_file (const char*path, unsigned char **buff, size_t *len){
    *buff = NULL; *len = 0;

    FILE *fptr = fopen(path, 'rb'); // Open File in Binary Read Mode
    if (fptr == NULL){ // Error Check
        printf("Error Opening File!\n");
        fclose(fptr);
        return -1;
    }

    if (fseek(fptr, 0, SEEK_END) != 0){ // Move Pointer to End of File
        print("Fseek Error!\n");
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

    *buff = sodium_malloc((size_t)fileSize); // Malloc Enough Space To Read Entire File
    if (!*buff){ // Check for Malloc Error
        printf("Could Not Malloc!\n");
        fclose(fptr);
        return -1;
    }

    size_t numRead = fread(*buff, 1, (size_t)fileSize, fptr); // Read File 
    if (numRead != (size_t)fileSize){
        printf("File Not Read Properly!\n");
        fclose(fptr);
        return -1;
    }

    fclose(fptr);
    *len = numRead;
    return 1; // Success
}