#include "dirent.h"
#include <sys/select.h>

DIR *opendir(const char *name) {};
struct dirent *readdir(DIR *dirp) {};
int closedir(DIR *dirp) { return -1; };

int lstat(const char *a, struct stat *b) { return -1; }

int select(int __n, fd_set *__readfds, fd_set *__writefds,
           fd_set *__exceptfds, struct timeval *__timeout) { return -1; }
