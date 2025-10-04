#include "../include/header.h"

/* read_file: read any file at `path` into a newly-allocated buffer.
   On success, sets *buff to sodium_malloc'd data and *len to its size.
   Returns 0 on success, -1 on failure. */
int read_file (const char*path, unsigned char **buff, size_t *len){
    *buff = NULL; *len = 0; // initialize outputs to safe defaults

    FILE *fptr = fopen(path, "rb"); // open file for binary reading
    // If open fails, report and bail.
    if (fptr == NULL){ // error check for fopen
        printf("Error Opening File!\n");
        return -1;
    }

    // Move the file position to the end to compute size.
    if (fseek(fptr, 0, SEEK_END) != 0){ // seek to end to measure size
        printf("Fseek Error!\n");
        fclose(fptr); // close file handle
        return -1;
    }

    long fileSize = ftell(fptr); // get current offset as file size
    // Validate size result.
    if (fileSize < 0){ // error check for ftell
        printf("File Does Not Exist\n");
        fclose(fptr); // close file handle
        return -1;
    }

    // Rewind back to the start for reading.
    if (fseek(fptr, 0, SEEK_SET) != 0){ // seek back to start
        printf("Fseek Error!\n"); // error check for rewind
        fclose(fptr); // close file handle
        return -1;
    }

    // Allocate buffer (at least 1 byte to avoid zero-sized allocation).
    if (fileSize > 0){
        *buff = sodium_malloc((size_t)fileSize); // allocate buffer for file content
        if (!*buff){ // check for allocation failure
            printf("Could Not Malloc!\n");
            fclose(fptr); // close file handle
            return -1;
        }
    } else {
        *buff = sodium_malloc(1); // allocate minimal buffer for empty file
        if (!*buff){ // check for allocation failure
            printf("Could Not Malloc!\n");
            fclose(fptr); // close file handle
            return -1;
        }
    }
    
    // Read the entire file content into the buffer.
    size_t numRead = fread(*buff, 1, (size_t)fileSize, fptr); // read file bytes
    if (numRead != (size_t)fileSize){ // verify full read
        printf("File Not Read Properly!\n");
        fclose(fptr); // close file handle
        sodium_free(*buff); // free allocated buffer on failure
        return -1;
    }

    fclose(fptr); // close file after successful read
    *len = numRead; // report number of bytes read
    return 0; // success
}


/* write_file: write `len` bytes from `buf` to a binary file at `path`.
   Overwrites or creates the file. Returns 0 on success, -1 on failure. */
int write_file(const char *path, const unsigned char *buf, size_t len){

    FILE* fptr = fopen(path, "wb"); // open file for binary writing
    // If open fails, bail.
    if (fptr == NULL){
        printf("Could Not Open File!\n");
        return -1;
    }

    size_t numWritten = fwrite(buf, 1, len, fptr); // write all bytes
    // Validate that the full buffer was written.
    if (numWritten != len){ // ensure complete write
        printf("Could Not Write To File!\n");
        fclose(fptr); // close file handle
        return -1;
    }

    fclose(fptr); // close on success
    return 0; // success
}

/* write_file_atomic_0600: atomically write `len` bytes from `buf` to `path`
   with final permissions 0600. Uses same-directory temp, fsync, and rename.
   Returns 0 on success, -1 on failure (errno set). */
int write_file_atomic_0600(const char *path, const unsigned char *buf, size_t len) {
    if (!path || (!buf && len != 0)) { errno = EINVAL; return -1; } // validate args

    /* Derive directory for the tmp file */
    char dir[PATH_MAX];
    const char *slash = strrchr(path, '/'); // find last forward slash
#ifdef _WIN32
    const char *bslash = strrchr(path, '\\'); // find last backslash on Windows
    if (!slash || (bslash && bslash > slash)) slash = bslash; // prefer later separator
#endif
    // Extract directory component or default to "."
    if (slash) {
        size_t dlen = (size_t)(slash - path); // compute directory length
        if (dlen == 0) {
            strcpy(dir, "/"); // root directory case
        } else if (dlen < sizeof(dir)) {
            memcpy(dir, path, dlen); // copy directory portion
            dir[dlen] = '\0'; // terminate dir string
        } else {
            errno = ENAMETOOLONG;
            return -1;
        }
    } else {
        strcpy(dir, "."); /* current directory */ // no separator → use "."
    }

    /* Build mkstemp template in same dir */
    char tmpl[PATH_MAX];
    if (snprintf(tmpl, sizeof tmpl, "%s/.streamseal.%ld.XXXXXX", dir, (long)getpid()) >= (int)sizeof tmpl) {
        errno = ENAMETOOLONG; return -1; // ensure template fits
    }

    /* Create temp file */
    int fd = mkstemp(tmpl); // create unique temp file
    if (fd < 0) return -1; // failure to create temp

    /* Force 0600 just in case umask restricted further or template created differently */
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) { int e = errno; close(fd); unlink(tmpl); errno = e; return -1; } // set perms 0600

    /* Write all bytes */
    size_t off = 0;
    // Loop: write until all bytes are written.
    while (off < len) {
        ssize_t w = write(fd, buf + off, len - off); // write next chunk
        if (w < 0) { int e = errno; close(fd); unlink(tmpl); errno = e; return -1; } // handle write error
        off += (size_t)w; // advance offset
    }

    /* Flush file contents to disk */
#if defined(__APPLE__)
    if (fcntl(fd, F_FULLFSYNC, 0) == -1) (void)fsync(fd); // macOS full fsync fallback
#else
    (void)fsync(fd); // ensure data hits disk
#endif
    if (close(fd) != 0) { int e = errno; unlink(tmpl); errno = e; return -1; } // close temp file

    /* Atomically replace target */
    if (rename(tmpl, path) != 0) { int e = errno; unlink(tmpl); errno = e; return -1; } // atomic rename over target

    /* Best-effort: fsync directory so the rename is durable */
    int dfd = open(dir, O_RDONLY); // open directory for fsync
    if (dfd >= 0) { (void)fsync(dfd); (void)close(dfd); } // sync and close dir fd

    /* Enforce final perms = 0600 (in case of inherited metadata) */
    (void)chmod(path, S_IRUSR | S_IWUSR); // set strict file mode

    return 0; // success
}

