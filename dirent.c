#include "dirent.h"
#include "fatfs/source/ff.h"
#include "sdcard.h"
#include "stdio_helper.h"
#include <pico/time.h>
#include <string.h>
#include <sys/select.h>

DIR *opendir(const char *name) {
    static DIR dir;
    f_opendir(&dir, name);
    return &dir;
}

struct dirent *readdir(DIR *dir) {
    static struct dirent entry;
    FILINFO info;
    FRESULT result = f_readdir(dir, &info);
    if (result != FR_OK || info.fname[0] == 0) {
        return NULL;
    }
    strcpy(entry.d_name, info.fname);
    return &entry;
}

int closedir(DIR *dir) {
    FRESULT result = f_closedir(dir);
    return result == FR_OK;
}

// TODO: use setStat from stdio_helper
int lstat(const char *filename, struct stat *filestat) {
    FILINFO fi;
    const FRESULT result = f_stat(filename, &fi);
    if (result != FR_OK) {
        return 1;
    }

    filestat->st_size = fi.fsize;
    filestat->st_atim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);
    filestat->st_mtim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);
    filestat->st_mode = S_IFREG;
    filestat->st_nlink = 1;
    return 0;
}
