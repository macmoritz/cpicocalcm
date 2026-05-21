#include "fatfs/source/ff.h"
#include <stdbool.h>
#include <sys/stat.h>

const char *adjust_filename(const char *fn);

bool hasError(FRESULT result);

// static inline void setStat(const FIL *file, struct stat *stat);
