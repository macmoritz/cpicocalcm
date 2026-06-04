#include "fatfs/source/ff.h"
#include <stdbool.h>
#include <sys/stat.h>

bool hasAndTranslateError(FRESULT result);

void setStat(const FIL *file, struct stat *stat);
