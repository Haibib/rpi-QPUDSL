#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    int file_descriptor = open(name, O_RDONLY);
    if (file_descriptor < 0) {
        output("file <%s> doesn't exist.\n", name);
        exit(1);
    }
    struct stat s;
    if (stat(name, &s) < 0 ) {
        close(file_descriptor);
        output("stat for <%s> failed.\n", name);
        exit(1);
    }
    *size = s.st_size;
    void* buffer = calloc(1, *size + (4 - *size % 4));
    if (*size <= 0) {
        close(file_descriptor);
        return buffer;
    }
    if (!buffer) {
        close(file_descriptor);
        output("calloc for <%s> failed.\n", name);
        exit(1);
    }
    read_exact(file_descriptor, buffer, *size);
    close(file_descriptor);
    return buffer;
}