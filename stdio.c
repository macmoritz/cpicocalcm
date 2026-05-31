#include "stdio_helper.h"
#include <fatfs/source/ff.h>
#include <fcntl.h>
#include <reent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/fcntl.h>

// tnylpo uses following file functions:
// regex used for search: `f[a-z]*\(`
// - fopen
// - fclose
// - ferror
// - feof
// - fstat
// - fwrite
// - fputwc
// - fgetwc
// - fgetws
// - fread
// - fprintf (overwrite/hook needed?)
// - fflush (overwrite/hook needed?)

#define LOG(...) printf("stdio.c: " __VA_ARGS__)

#define FILE_DESCRIPTOR_MIN 3 // 0/1/2 are stdin/stdout/stderr
#define MAX_FILES_OPEN 5

FIL openFiles[MAX_FILES_OPEN];
bool fileInUse[MAX_FILES_OPEN];

static inline int getFILIndex(int file) {
    int i = file - FILE_DESCRIPTOR_MIN;
    if (i < 0 || i >= MAX_FILES_OPEN) {
        errno = EINVAL;
        return -1;
    }
    if (!fileInUse[i]) {
        errno = EINVAL;
        return -1;
    }
    return i;
}

int _open(const char *filename, int oflag) {
    int slot = 0;
    bool foundFreeSlot = false;
    for (; slot < MAX_FILES_OPEN; slot++) {
        if (!fileInUse[slot]) {
            fileInUse[slot] = true;
            foundFreeSlot = true;
            break;
        }
    }
    if (!foundFreeSlot) {
        errno = ENFILE;
        return -1;
    }

    BYTE mode = 0;
    switch (oflag & O_ACCMODE) {
    case O_RDONLY:
        mode |= FA_READ;
        break;
    case O_WRONLY:
        mode |= FA_WRITE;
        break;
    case O_RDWR:
        mode |= FA_READ | FA_WRITE;
        break;
    default:
        mode |= FA_READ;
        break;
    }
    if (oflag & O_CREAT) {
        if (oflag & O_EXCL) {
            mode |= FA_CREATE_NEW; // mode: wx, w+x
        } else if (oflag & O_TRUNC) {
            mode |= FA_CREATE_ALWAYS; // mode: w, wx
        } else {
            mode |= FA_OPEN_ALWAYS;
        }
    } else {
        mode |= FA_OPEN_EXISTING;
    }
    if (oflag & O_APPEND) {
        mode &= ~FA_OPEN_EXISTING; // unset
        mode &= ~FA_OPEN_ALWAYS;   // unset
        mode |= FA_OPEN_APPEND;
    }

    FRESULT result = f_open(&openFiles[slot], filename, mode);
    // LOG("_open {fn: %s; oflag: %d; mode: %d}\tresult: %d\n", filename, oflag, mode, result);
    if (hasError(result)) {
        fileInUse[slot] = false;
        return -1;
    }

    return FILE_DESCRIPTOR_MIN + slot;
}

int _close(int file) {
    int fileIndex = getFILIndex(file);
    if (fileIndex == -1) {
        return -1;
    }
    const FRESULT result = f_close(&openFiles[fileIndex]);
    if (hasError(result)) {
        return -1;
    }
    fileInUse[fileIndex] = false;
    return 0;
}

int _read(int fd, char *buffer, int length) {
    int fileIndex = getFILIndex(fd);
    if (fileIndex == -1) {
        return -1;
    }
    UINT bytesRead = 0;
    const FRESULT result = f_read(&openFiles[fileIndex], buffer, length, &bytesRead);
    if (hasError(result)) {
        return -1;
    }
    return bytesRead;
}

off_t _lseek(int fd, off_t pos, int whence) {
    int fileIndex = getFILIndex(fd);
    if (fileIndex == -1) {
        return -1;
    }
    const FIL *fp = &openFiles[fileIndex];
    FSIZE_t newPos = pos;
    if (whence == SEEK_CUR) {
        newPos += f_tell(fp);
    } else if (whence == SEEK_END) {
        newPos += f_size(fp);
    }

    const FRESULT result = f_lseek(&openFiles[fileIndex], pos);
    if (hasError(result)) {
        return -1;
    }
    return pos;
}

static inline void setStat(const FIL *file, struct stat *stat) {
    memset(stat, 0, sizeof(*stat));
    stat->st_size = f_size(file);
    stat->st_mode = S_IFREG;
    stat->st_nlink = 1;
}

int _fstat(int fd, struct stat *buf) {
    int fileIndex = getFILIndex(fd);
    if (fileIndex == -1) {
        return -1;
    }
    const FIL *fp = &openFiles[fileIndex];
    setStat(fp, buf);

    return 0;
}

int _stat(const char *file, struct stat *buf) {
    FIL *f;
    const FRESULT result = f_open(f, file, FA_READ | FA_OPEN_EXISTING);
    if (hasError(result)) {
        return -1;
    }
    setStat(f, buf);
    f_close(f);
    return 0;
}

int _write(int fd, char *buffer, int length) {
    if (length < 0) {
        errno = EINVAL;
        return -1;
    }

    int fileIndex = getFILIndex(fd);
    if (fileIndex == -1) {
        return -1;
    }

    // LOG("_write {fd: %d, buffer: %c%c%c..., length: %d}", fileIndex, buffer[0], buffer[1], buffer[2], length);

    UINT bytesWritten = 0;
    const FRESULT result = f_write(&openFiles[fileIndex], buffer, length, &bytesWritten);
    // LOG("\tbytesToWrite: %d\t bytesWritten: %d\n", length, bytesWritten);
    if (hasError(result)) {
        // LOG("_write failed: %d\n", result);
        return -1;
    }
    return bytesWritten;
}

int _rename(const char *old, const char *new) {
    const FRESULT result = f_rename(old, new);
    // LOG("_rename {old: %s; new: %s; status: %d}\n", old, new, result);
    if (hasError(result)) {
        return -1;
    }
    return 0;
}

// self copying is restricted by file open modifiers
int _link(const char *old, const char *new) {
    FIL fold;
    FIL fnew;

    FRESULT result = f_open(&fold, old, FA_READ | FA_OPEN_EXISTING);
    if (hasError(result)) {
        return -1;
    }
    result = f_open(&fnew, new, FA_WRITE | FA_CREATE_NEW);
    if (hasError(result)) {
        return -1;
    }
    uint8_t buffer[512];
    UINT bytesread, byteswritten;
    for (int i = 0; i < f_size(&fold); i += 512) {
        result = f_read(&fold, &buffer, 512, &bytesread);
        if (hasError(result)) {
            return -1;
        }
        result = f_write(&fnew, &buffer, bytesread, &byteswritten);
        if (hasError(result)) {
            return -1;
        }
    }
    result = f_close(&fold);
    if (hasError(result)) {
        return -1;
    }
    result = f_close(&fnew);
    if (hasError(result)) {
        return -1;
    }
    return 0;
}

int _unlink(const char *filename) {
    const FRESULT result = f_unlink(filename);
    if (hasError(result)) {
        return -1;
    }
    return 0;
}