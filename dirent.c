#include "dirent.h"
#include "fatfs/source/ff.h"
#include "sdcard.h"
#include "stdio_helper.h"
#include <pico/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

DIR *opendir(const char *name) {
    static DIR dir;
    // TODO: Check for following
    // TODO: Use stdio_helper translation
    if (name[0] == '.') {
        name++;
    }
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
    if (entry.d_name == NULL) {
        const size_t fname_len = strlen(info.fname) + 1;
        entry.d_name = malloc(fname_len);
        if (entry.d_name == NULL) {
            return NULL;
        }
    }
    strcpy(entry.d_name, info.fname);
    return &entry;
}

int closedir(DIR *dir) {
    FRESULT result = f_closedir(dir);
    return result == FR_OK;
}

// TODO: use setStat from stdio_helper
int lstat(const char *path, struct stat *filestat) {
    FILINFO fi;
    const char *filename = adjust_filename(path);
    const FRESULT result = f_stat(filename, &fi);
    printf("lstat {path: %s; result: %d, fsize: %d}\n", path, result, fi.fsize);
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
