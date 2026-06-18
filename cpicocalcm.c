#include "fatfs/source/ff.h"
#include "sdcard.h"
#include <pico/platform/common.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define _POSIX_TIMERS 1

#include "cpm-tpa/assembly/bell.h"
#include "cpm-tpa/assembly/keyboard_bell.h"
// #include "cpm-tpa/bell.h"
// #include "cpm-tpa/keyboard_bell.h"
#include "cpm-tpa/ncurses_test.h"
#include "lcd.h"
#include "ncurses.h"
#include "picocalc.h"
#include "stdio_helper.h"
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

static atomic_bool core1_running = true;
static atomic_bool core1_stopped = false;
char *tnylpo_config = ".tnylpo.conf";
struct repeating_timer contentBlinkTimer;

bool contentBlinkCallback(struct repeating_timer *timer) {
    WINDOW *scr = timer->user_data;
    if (scr == NULL) {
        return false;
    }
    scr->blinkstate = !scr->blinkstate;
    return true;
}

// This runs on the second core and renders the video content as well as sending the rendered content to the lcd
void lcdJob() {
    lcd_init();
    lcd_clear();
    add_repeating_timer_ms(-500, contentBlinkCallback, displayscr, &contentBlinkTimer);
    uint64_t frameCounter = 0;
    uint64_t frameStart = to_us_since_boot(get_absolute_time());
    buffer0 = malloc(bufferSize * sizeof(COLOR_TYPE));
    buffer1 = malloc(bufferSize * sizeof(COLOR_TYPE));
    bool stopInNextIteration = false;
    while (1) {
        lcd_update(displayscr);
        frameCounter++;
        if (frameCounter == 100) {
            const int64_t delta = absolute_time_diff_us(frameStart, to_us_since_boot(get_absolute_time()));
            const double averageTimePerFrameUs = (double)delta / (double)frameCounter;
            const double framesPerSecond = 1000000 / averageTimePerFrameUs;
            printf("Frames per Second: %f\n", framesPerSecond);
            frameStart = to_us_since_boot(get_absolute_time());
            frameCounter = 0;
        }
        if (stopInNextIteration) {
            break;
        }
        if (!atomic_load_explicit(&core1_running, memory_order_acquire)) {
            stopInNextIteration = true;
        }
    }
    cancel_repeating_timer(&contentBlinkTimer);
    free(buffer0);
    free(buffer1);
    delwin(displayscr); // free content
    delwin(displayscr); // free WINDOW
    atomic_store_explicit(&core1_stopped, true, memory_order_release);
}

// Cleanup the environment.
static inline void cleanup() {
    f_unmount("");
    multicore_reset_core1();
}

/**
 * @brief Reads file '.cpicocalcm.conf', parses and sets configuration values.
 * Format is inspired by tnylpo:
 * Empty lines and lines starting with a hash sign (#) or a semicolon (;) are ignored.
 * All other lines have the form
 *
 *   <keyword> = <token>
 *
 * <keyword> is a lowercase string.
 * <token> is a string in double quotes.
 *
 * @return -1 if an error occured, any number equal or greater 0 is the count of parameters set.
 */
int readCPicoCalcMConfig() {
    FIL file;
    FRESULT result = f_open(&file, ".cpicocalcm.conf", FA_READ);
    if (hasAndTranslateError(result)) {
        return -1;
    }
    int count = 0;
    char *buffer = malloc(255 * sizeof(char));
    while (!f_eof(&file)) {
        f_gets(buffer, 255, &file);

        char *start = buffer;
        while (*start < 'a' && *start > 'z' && *start != ';' && *start != '#' && *start != '\n') {
            start++;
        }
        if (*start == ';' || *start == '#' || *start == '\n') {
            // line is empty or starts with ';' or '#'
            continue;
        }

        char *value = buffer;
        while (*value != '=' && *value != '\n') {
            value++;
        }
        value++; // step over '='
        if (*value == '\n') {
            // no value given
            return -1;
        }
        while (*value == '\t' || *value == ' ' || *value == '"' || *value == '\'') {
            value++;
        }
        int valueLength = 0;
        for (int i = 0; value[i] >= '.' && value[i] <= 'z'; i++) {
            valueLength++;
        }

        char **target = NULL;
        if (strncmp(start, "command", 7) == 0) {
            target = &conf_command;
        } else if (strncmp(start, "tnylpo config", 13) == 0) {
            free(tnylpo_config);
            target = &tnylpo_config;
        }

        if (target != NULL) {
            *target = malloc((valueLength + 1) * sizeof(char));
            if (*target == NULL) {
                return -1;
            }
            memcpy(*target, value, valueLength * sizeof(char));
            (*target)[valueLength] = '\0';
            count += 1;
        }
    }

    return count;
}

int main() {
    stdio_init_all();

    const struct timespec ts = {0, 0};
    aon_timer_start(&ts);

    picocalc_init();
    picocalc_drain_keyboard_fifo();
    picocalc_print_version();
    picocalc_read_battery();

    FRESULT fresult;
    FATFS fatfs;
    fresult = f_mount(&fatfs, "", 1);
    if (fresult != FR_OK) {
        panic("f_mount failed: %d\n", fresult);
    }

    // conf_command = "./all.com"; // Emulated CPU Speed: 6.094598 MHz, 6094598.100088 Hz
    // conf_command = "./heap.com"; // Emulated CPU Speed: 6.809115 MHz, 6809114.745804 Hz

    // Configuring portation (f.e. path to executable file, path to tnylpo config)
    int status = readCPicoCalcMConfig();
    if (status == -1) {
        printf("readCPicoCalcMConfig failed\n");
        cleanup();
        return 1;
    }

    // Recommended configuration:
    conf_color = true;
    conf_background = COLOR_BLACK;
    conf_foreground = COLOR_WHITE;
    conf_memsize = 0; // default memory configuration

    status = read_config(tnylpo_config);
    if (status) {
        printf("read_config failed\n");
        cleanup();
        return 1;
    }

    // Persistent configuration:
    cols = lines = 40;       // 40x40 chars with 8x8 pixel chars = 320x320 pixel output
    screen_delay = 0;        // no delay, exit emulation directly
    conf_interactive = true; // ncurses "graphical" output
    dont_close = false;      // default value, close file if closed in emulated software

    // stdio_init_all();
    // cpu_init();
    // memory[0] = 0x3e; // LD A, 0xff
    // memory[1] = 0xff;
    // memory[2] = 0x47; // LD B, A
    // memory[3] = 0x05; // DEC B
    // memcpy(memory + TPA_START, bell_com, bell_com_len);
    // memcpy(memory + TPA_START, keyboard_bell_com, keyboard_bell_com_len);
    // memcpy(memory + TPA_START, colors_com, colors_com_len);
    // memcpy(memory + TPA_START, ncurses_test_com, ncurses_test_com_len);
    // cpu_run();
    // printf("Registers: A: 0x%02x\tB: 0x%02x\n", reg_a, reg_b); // Registers: A: 0xff      B: 0xfe

    status = cpu_init();
    if (status) {
        printf("cpu_init failed\n");
        cleanup();
        return 1;
    }

    status = console_init();
    if (status) {
        printf("console_init failed\n");
        cleanup();
        return 1;
    }
    multicore_launch_core1(lcdJob);
    cpu_run();
    wrefresh(curscr);
    atomic_store_explicit(&core1_running, false, memory_order_release);

    while (!atomic_load_explicit(&core1_stopped, memory_order_acquire)) {
        tight_loop_contents();
    }

    status = console_exit();
    if (status) {
        printf("console_exit failed\n");
        return 1;
    }
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

    // const struct timespec ts = {0, 0};
    // aon_timer_start(&ts);
    // sleep_ms(2000);
    // struct tm tm;
    // aon_timer_get_time_calendar(&tm);
    // printf("tm sec: %d\n", tm.tm_sec);

    cleanup();
    exit(0);
}
