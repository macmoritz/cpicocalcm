#ifndef DIRENT_H
#define DIRENT_H

#include "fatfs/source/ff.h"
#include <sys/stat.h>

struct dirent {
    char *d_name;
};

DIR *opendir(const char *name);

struct dirent *readdir(DIR *dirp);

int closedir(DIR *dir);

int lstat(const char *path, struct stat *filestat);

#endif