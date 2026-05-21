#include <pico/time.h>
#include <sys/select.h>

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout) {
    sleep_us(__timeout->tv_usec + __timeout->tv_sec * 1000000000);
    return 0;
}
