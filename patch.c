#include "tnylpo/tnylpo.h"
#include <pico/aon_timer.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/_timespec.h>
#include <sys/_timeval.h>
#include <sys/signal.h>

void perr(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void plog(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void plog_dump(int addr, int length) {}

int sigaction(int a, const struct sigaction *b, struct sigaction *c) {
    return -1;
}

int nanosleep(const struct timespec *requested, struct timespec *remaining) {
    sleep_us(requested->tv_nsec / 1000 + requested->tv_sec * 1000000);
    return 0;
}
