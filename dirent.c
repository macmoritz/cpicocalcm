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

int lstat(const char *filename, struct stat *filestat) {
    FIL f;
    FRESULT result = f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
    if (hasAndTranslateError(result)) {
        return 1;
    }
    setStat(&f, filestat);
    f_close(&f);

    FILINFO fi;
    result = f_stat(filename, &fi);
    if (result != FR_OK) {
        return 1;
    }

    filestat->st_atim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);
    filestat->st_mtim = get_seconds_since_epoch_from_fattime(fi.fdate, fi.ftime);
    return 0;
}
