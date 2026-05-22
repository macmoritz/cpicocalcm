#include "fatfs/source/ff.h"
#include "sdcard.h"
#include <pico/platform/common.h>
#include <stdatomic.h>
#include <stdlib.h>
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

static atomic_bool core1_running = true;
static atomic_bool core1_stopped = false;

// This runs on the second core and renders the video content as well as sending the rendered content to the lcd
void lcdJob() {
    lcd_init();
    lcd_clear();
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

    // f_unlink("/ers.tmp");
    // f_unlink("/old.tmp");
    // f_unlink("/new.tmp");
    // f_unlink("/raw.tmp");
    // f_unlink("/animals.dat");
    // stdio.c: _open {fn: /animals.dat; oflag: 2562; mode: 7} result: 8
    // make file (FCB 0x2e13): could not create ./animals.dat: File exists
    // 2562 = 0xa02 = 0x0800 + 0x0200 + 0x0002 = FEXCL (error on open if file exists) + FCREATE (open with file create) + FRDWR (Read + Write)
    // animals calls dbos 19 -> delete file
    // TODO: does this work in fatfs?

    // Read dir
    printf("SD Card Content:\n");
    DIR dir;
    FILINFO fileinfo;
    fresult = f_opendir(&dir, "/");
    if (fresult != FR_OK) {
        cleanup();
        panic("f_opendir failed: %d\n", fresult);
    }

    while (1) {
        fresult = f_readdir(&dir, &fileinfo);
        if (fresult != FR_OK || fileinfo.fname[0] == 0) {
            break;
        }
        printf("\t%s %c\n", fileinfo.fname, (fileinfo.fattrib & AM_DIR) ? '/' : ' ');
    }
    f_closedir(&dir);
    printf("\n");

    // FIL file;
    // f_open(&file, "test.txt", FA_READ);
    // char *buffer = calloc(20, sizeof(char));
    // UINT bytes_read = 0;
    // f_read(&file, buffer, 20, &bytes_read);
    // buffer[19] = '\0';
    // printf("test.txt content: %s\n", buffer);
    // f_close(&file);

    // fresult = f_open(&file, "fatfs.txt", FA_OPEN_APPEND | FA_WRITE);
    // if (fresult != FR_OK) {
    //     panic("f_open FA_CREATE_NEW failed: %d\n", fresult);
    // }
    // buffer = "String aus D";
    // UINT written = 0;
    // fresult = f_write(&file, buffer, 13, &written);
    // if (fresult != FR_OK) {
    //     panic("f_write failed: %d\n", fresult);
    // }
    // printf("Written %d bytes\n", written);
    // f_close(&file);
    // f_unmount("");
    // return 0;

    conf_command = "/files.com";
    // conf_command = "/animals.com";
    // conf_command = "/all.com";
    conf_color = true;
    conf_background = COLOR_BLACK;
    conf_foreground = COLOR_WHITE;
    // int status = read_config(NULL); // needed to set charset, by default VT52
    // TODO: Check behaviour (esp. charset) when file not found
    int status = read_config(".tnylpo.conf");
    if (status) {
        printf("read_config failed\n");
        cleanup();
        return 1;
    }

    // Persistent configuration:
    cols = lines = 40; // 40x40 chars with 8*8 pixel chars = 320x320 pixel output
    screen_delay = 0;  // no delay, exit emulation directly
    // screen_delay = -1; // delay, wait to exit emulation
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

    while (!atomic_load_explicit(&core1_stopped, memory_order_acquire)) {
        tight_loop_contents();
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
