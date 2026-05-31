#include "stdio_helper.h"
#include "fatfs/source/ff.h"
#include <stdbool.h>
#include <sys/errno.h>
#include <sys/stat.h>

bool hasAndTranslateError(FRESULT result) {
    switch (result) {
    case FR_OK: /* Function succeeded */
        return false;
    case FR_DISK_ERR: /* A hard error occurred in the low level disk I/O layer */
        errno = EIO;
        break;
    case FR_INT_ERR: /* Assertion failed */
        errno = ECANCELED;
        break;
    case FR_NOT_READY: /* The physical drive does not work */
        errno = EBUSY;
        break;
    case FR_NO_FILE:      /* Could not find the file */
    case FR_NO_PATH:      /* Could not find the path */
    case FR_INVALID_NAME: /* The path name format is invalid */
        errno = ENOENT;
        break;
    case FR_DENIED: /* Access denied due to a prohibited access or directory full */
        errno = EACCES;
        break;
    case FR_EXIST: /* Access denied due to a prohibited access */
        errno = EEXIST;
        break;
    case FR_INVALID_OBJECT: /* The file/directory object is invalid */
        errno = ENOENT;
        break;
    case FR_WRITE_PROTECTED: /* The physical drive is write protected */
        errno = EROFS;
        break;
    case FR_INVALID_DRIVE: /* The logical drive number is invalid */
    case FR_NOT_ENABLED:   /* The volume has no work area */
    case FR_NO_FILESYSTEM: /* Could not find a valid FAT volume */
        errno = ENXIO;
        break;
    case FR_MKFS_ABORTED: /* The f_mkfs function aborted due to some problem */
        errno = EIO;
        break;
    case FR_TIMEOUT: /* Could not take control of the volume within defined period */
        errno = EBUSY;
        break;
    case FR_LOCKED: /* The operation is rejected according to the file sharing policy */
        errno = EACCES;
        break;
    case FR_NOT_ENOUGH_CORE: /* LFN working buffer could not be allocated, given buffer size is insufficient or too deep path */
        errno = ENAMETOOLONG;
        break;
    case FR_TOO_MANY_OPEN_FILES: /* Number of open files > FF_FS_LOCK */
        errno = ENFILE;
        break;
    case FR_INVALID_PARAMETER: /* Given parameter is invalid */
        errno = EINVAL;
        break;
    }

    return true;
}

// static inline void setStat(const FIL *file, struct stat *stat) {
//     memset(stat, 0, sizeof(*stat));
//     stat->st_size = f_size(file);
//     stat->st_mode = S_IFREG;
//     stat->st_nlink = 1;

//     FILINFO fi;
//     const FRESULT result = f_stat(file->, &fi);
//     printf("lstat {path: %s; result: %d, fsize: %d}\n", path, result, fi.fsize);
//     if (result != FR_OK) {
//         return 1;
//     }
//     filestat->st_size = fi.fsize;
//     filestat->st_atim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);
//     filestat->st_mtim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);

// }
