#include <string.h>
#define _POSIX_TIMERS 1

// #include "tnylpo/cpu.c"
// #include "tnylpo/os.c"
#include "cpm-tpa/assembly/bell.h"
#include "cpm-tpa/assembly/keyboard_bell.h"
// #include "cpm-tpa/bell.h"
// #include "cpm-tpa/keyboard_bell.h"
#include "cpm-tpa/ncurses_test.h"
#include "lcd.h"
#include "ncurses.h"
#include "picocalc.h"
#include "tnylpo/tnylpo.h"
#include <hardware/gpio.h>
#include <pico/aon_timer.h>
#include <pico/multicore.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

#define TPA_START 0x0100

// This runs on the second core and renders the video content as well as sending the rendered content to the lcd
void lcdJob() {
    lcd_init();
    lcd_clear();
    uint64_t frameCounter = 0;
    uint64_t frameStart = to_us_since_boot(get_absolute_time());
    while (1) {
        lcd_update(curscr);
        frameCounter++;
        if (frameCounter == 100) {
            const int64_t delta = absolute_time_diff_us(frameStart, to_us_since_boot(get_absolute_time()));
            const double averageTimePerFrameUs = (double)delta / (double)frameCounter;
            const double framesPerSecond = 1000000 / averageTimePerFrameUs;
            printf("Frames per Second: ~%.2f\n", framesPerSecond);
            frameStart = to_us_since_boot(get_absolute_time());
            frameCounter = 0;
        }
    }
}

int main() {
    stdio_init_all();
    const struct timespec ts = {0, 0};
    aon_timer_start(&ts);

    picocalc_init();
    picocalc_drain_keyboard_fifo();
    picocalc_print_version();
    picocalc_read_battery();

    screen_delay = 0;            // no delay, exit emulation directly
    conf_command = "./main.com"; // Does not get loaded anyway
    conf_interactive = true;
    conf_background = COLOR_BLACK;
    conf_foreground = COLOR_WHITE;
    int status = read_config(NULL); // needed to set charset, by default VT52
    if (status) {
        printf("read_config failed\n");
        return 1;
    }
    cols = lines = 40; // 40x40 chars with 8*8 pixel chars = 320x320 pixel output

    status = cpu_init();
    if (status) {
        printf("cpu_init failed\n");
        return 1;
    }
    // memory[0] = 0x3e; // LD A, 0xff
    // memory[1] = 0xff;
    // memory[2] = 0x47; // LD B, A
    // memory[3] = 0x05; // DEC B
    // memcpy(memory + TPA_START, bell_com, bell_com_len);
    // memcpy(memory + TPA_START, keyboard_bell_com, keyboard_bell_com_len);
    // memcpy(memory + TPA_START, colors_com, colors_com_len);
    memcpy(memory + TPA_START, ncurses_test_com, ncurses_test_com_len);

    status = console_init();
    if (status) {
        printf("cpu_init failed\n");
        return 1;
    }
    multicore_launch_core1(lcdJob);
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
    status = console_exit();
    if (status) {
        printf("console_exit failed\n");
        return 1;
    }

    // const struct timespec ts = {0, 0};
    // aon_timer_start(&ts);
    // sleep_ms(2000);
    // struct tm tm;
    // aon_timer_get_time_calendar(&tm);
    // printf("tm sec: %d\n", tm.tm_sec);

    multicore_reset_core1();
    exit(0);
}
