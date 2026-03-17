#pragma once

#include <sys/_timespec.h>

int nanosleep(const struct timespec *__requested_time, struct timespec *__remaining);