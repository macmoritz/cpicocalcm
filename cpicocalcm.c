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
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

#define TPA_START 0x0100

int main() {
    stdio_init_all();
    const struct timespec ts = {0, 0};
    aon_timer_start(&ts);

    picocalc_init();
    picocalc_drain_keyboard_fifo();
    picocalc_print_version();
    picocalc_read_battery();

    lcd_init();
    lcd_clear();
    // const uint64_t time = to_us_since_boot(get_absolute_time());
    // const int colors[8] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
    // const char chars[8] = {'R', 'A', 'I', 'N', 'B', 'O', 'W', '!'};
    // for (int b = 0; b < 8; b++) {
    //     for (int f = 0; f < 8; f++) {
    //         lcd_print_char(chars[f], colors[f], colors[b], f * 8, b * 8, false);
    //     }
    // }

    // // draw_rect_spi(0, 0, 320, 320, COBALT);
    // const int64_t delta = absolute_time_diff_us(time, to_us_since_boot(get_absolute_time()));
    // printf("vBlank stuff took %lld us\n", delta);

    screen_delay = 0;            // no delay, exit emulation directly
    conf_command = "./main.com"; // Does not get loaded anyway
    conf_interactive = true;
    conf_background = COLOR_BLUE;
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
    memcpy(memory + TPA_START, ncurses_test_com, ncurses_test_com_len);

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

    exit(0);
}
