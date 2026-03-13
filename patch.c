#include "tnylpo/tnylpo.h"
#include <pico/aon_timer.h>
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

void usage(void) {}

void os_call(int magic) {}

void console_poll(void) {}

// implementing sigaction for
// /home/ubuntu/.pico-sdk/toolchain/14_2_Rel1/arm-none-eabi/include/sys/signal.h
int sigaction(int a, const struct sigaction *b, struct sigaction *c) {
    return -1;
}

// implementing gettimeofday
int gettimeofday(struct timeval *__restrict __p, void *__restrict __tz) {
    absolute_time_t time = aon_timer_get_time((struct timespec *)__p);
    return 0;
}

int nanosleep(const struct timespec *__requested_time, struct timespec *__remaining) {
    sleep_us(__requested_time->tv_nsec / 1000);
    return 0;
}