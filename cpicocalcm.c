#define _POSIX_TIMERS 1

#include "tnylpo/cpu.c"
#include "tnylpo/tnylpo.h"
#include <hardware/gpio.h>
#include <pico/aon_timer.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

int main() {
    stdio_init_all();

    cpu_init();
    memory[0] = 0x3e; // LD A, 0xff
    memory[1] = 0xff;
    memory[2] = 0x47; // LD B, A
    memory[3] = 0x05; // DEC B
    cpu_run();

    const struct timespec ts = {0, 0};
    aon_timer_start(&ts);
    sleep_ms(2000);
    struct tm tm;
    aon_timer_get_time_calendar(&tm);
    printf("tm sec: %d\n", tm.tm_sec);

    exit(0);
}
