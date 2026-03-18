#include <string.h>
#define _POSIX_TIMERS 1

// #include "tnylpo/cpu.c"
// #include "tnylpo/os.c"
#include "cpm-tpa/bell.h"
#include "tnylpo/tnylpo.h"
#include <hardware/gpio.h>
#include <pico/aon_timer.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

#define TPA_START 0x0100

int main() {
    stdio_init_all();

    conf_command = "./main.com"; // Does not get loaded anyway
    int status = cpu_init();
    if (status) {
        printf("cpu_init failed\n");
        return 1;
    }
    // memory[0] = 0x3e; // LD A, 0xff
    // memory[1] = 0xff;
    // memory[2] = 0x47; // LD B, A
    // memory[3] = 0x05; // DEC B
    memcpy(memory + TPA_START, bell_com, bell_com_len);

    status = console_init();
    if (status) {
        printf("cpu_init failed\n");
        return 1;
    }
    cpu_run();
    status = cpu_exit();
    if (status) {
        printf("cpu_exit failed\n");
        return 1;
    }
    status = finalize_chario();
    if (status) {
        printf("finalize_chario failed\n");
        return 1;
    }
    // status = console_exit();
    // if (status) {
    //     printf("console_exit failed\n");
    //     return 1;
    // }

    const struct timespec ts = {0, 0};
    aon_timer_start(&ts);
    sleep_ms(2000);
    struct tm tm;
    aon_timer_get_time_calendar(&tm);
    printf("tm sec: %d\n", tm.tm_sec);

    exit(0);
}
