#pragma once

// clang-format off
#include <utilities.h>
// clang-format on

static inline unsigned char* __open(const char* const fpath, long* const nreadbytes) {
    *nreadbytes             = 0;
    unsigned char* buffer   = 0;
    struct stat    filestat = { 0 };
    long           nbytes   = 0;

    const int fdesc         = open(fpath, O_RDONLY);
    if (fdesc == -1) { // if open() failed, the return value will be -1
        fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        return nullptr;
    }

    if (fstat(fdesc, &filestat)) { // if succeeds, 0 is returned, -1 if fails
        fprintf(stderr, "Call to fstat() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        goto CLOSE_AND_RETURN;
    }

    if (!(buffer = malloc(filestat.st_size))) { // caller is responsible for freeing this buffer
        fprintf(stderr, "Call to new() failed inside %s at line %d!\n", __FUNCTION__, __LINE__);
        goto CLOSE_AND_RETURN;
    }

    if ((nbytes = read(fdesc, buffer, filestat.st_size)) != -1) {
        *nreadbytes = nbytes;
        assert(nbytes == filestat.st_size); // double checking
    } else {
        fprintf(stderr, "Call to read() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        free(buffer);
        buffer = nullptr;
    }
    // then, fall through the CLOSE_AND_RETURN label

CLOSE_AND_RETURN:
    // close() returns 0 on success and -1 on failure
    if (close(fdesc)) fprintf(stderr, "Call to close() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
    return buffer;
}

// a file format agnostic write routine to serialize binary image files, if a file with the specified name exists on disk, it will be overwritten
static inline bool __write(const char* const filename, const unsigned char* const buffer, const long* const size) {
    assert(filename); // too much??
    if (!buffer) {
        fprintf(stderr, "Empty buffer passed to function %s at line %d\n", __FUNCTION__, __LINE__);
        return false; // fail if the buffer is a nullptr
    }

    bool      is_success = false; // has every step succeeded???
    long      nbytes     = 0;     // number of bytes serialized to the disk
    const int fdesc      = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IROTH | S_IWUSR | S_IWOTH);
    // without explicitly specifying the mode_t, we had to use sudo to open the written images, open the file descriptor with create and write privileges

    if (fdesc == -1) {
        fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        goto CLOSE_AND_RETURN;
    }

    nbytes = write(fdesc, buffer, size);
    if (nbytes == -1) {
        fprintf(stderr, "Call to write() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        goto CLOSE_AND_RETURN;
    }

    // if the write was successful,
    is_success = size == nbytes;
    // then, fall through the CLOSE_AND_RETURN label

CLOSE_AND_RETURN:
    close(fdesc);
    return is_success;
}
