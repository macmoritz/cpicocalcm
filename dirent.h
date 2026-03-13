#ifndef DIRENT_H
#define DIRENT_H

typedef struct {
} DIR;

struct dirent {
    char *d_name;
};

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

int lstat(const char *, struct stat *) { return -1; }

#endif