#include "picocalc.h"
#include "keys.h"
#include <hardware/clocks.h>
#include <hardware/i2c.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>

void picocalc_init() {
    // Audio init
    gpio_init(AUDIO_PWM_L);
    gpio_init(AUDIO_PWM_R);
    gpio_set_function(AUDIO_PWM_L, GPIO_FUNC_PWM);
    gpio_set_function(AUDIO_PWM_R, GPIO_FUNC_PWM);

    // Keyboard init
    gpio_set_function(I2C_KBD_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C_KBD_SDA, GPIO_FUNC_I2C);
    i2c_init(I2C_KBD_MOD, I2C_KBD_SPEED);
    gpio_pull_up(I2C_KBD_SCL);
    gpio_pull_up(I2C_KBD_SDA);
}

void picocalc_beep(uint32_t freq, uint32_t duration) {
    const uint slice = pwm_gpio_to_slice_num(AUDIO_PWM_L);
    const uint channelL = pwm_gpio_to_channel(AUDIO_PWM_L);
    const uint channelR = pwm_gpio_to_channel(AUDIO_PWM_R);

    const uint level = 1000;
    const uint32_t clk = clock_get_hz(clk_sys);
    const float clkdiv = (float)clk / ((float)freq * level);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, level);
    pwm_config_set_clkdiv(&cfg, clkdiv);
    pwm_init(slice, &cfg, true);

    pwm_set_chan_level(slice, channelL, level / 2);
    pwm_set_chan_level(slice, channelR, level / 2);

    sleep_ms(duration);

    pwm_set_chan_level(slice, channelL, 0);
    pwm_set_chan_level(slice, channelR, 0);
    pwm_set_enabled(slice, false);
}

// https://github.com/clockworkpi/PicoCalc/blob/master/Code/picocalc_kbd_tester/i2ckbd/i2ckbd.c
int picocalc_read_i2c_kbd() {
    // returned data: [0] keystate, [1] keycode
    // keystate:
    //  - 0: IDLE
    //  - 1: Pressed
    //  - 2: Hold
    //  - 3: Released
    static int ctrlheld = 0;
    uint8_t buf[2] = {0xff, 0};
    int status = picocalc_i2c(0x09, buf);
    if (status == -1) {
        printf("read_i2c_keyboard i2c write error\n");
        return -1;
    } else if (status == -2) {
        printf("read_i2c_keyboard i2c read error\n");
        return -1;
    }

    if (buf[0] == 1) { // keystate: pressed
        return buf[1]; // keycode
    }

    return -1;
}

// https://github.com/clockworkpi/PicoCalc/blob/master/Code/picocalc_kbd_tester/i2ckbd/i2ckbd.c
int picocalc_read_battery() {
    // returned data: [0] reg, [1] reg value: battery in percent
    uint8_t buf[2] = {0xff, 0};

    int status = picocalc_i2c(0x0b, buf);
    if (status == -1) {
        printf("read_battery i2c write error\n");
        return -1;
    } else if (status == -2) {
        printf("read_battery i2c read error\n");
        return -1;
    }

    if (buf[0] != 0xff) {
        printf("Battery: %d%%\n", buf[1]);
        return buf[1];
    }
    return -1;
}

void picocalc_print_version() {
    // returned data: [0] 0x00, [1] contains version
    uint8_t buf[2] = {0xff, 0};

    int status = picocalc_i2c(0x01, buf);
    if (status == -1) {
        printf("print_version i2c write error\n");
        return;
    } else if (status == -2) {
        printf("print_version i2c read error\n");
        return;
    }

    if (buf[0] == 0x00) {
        printf("PicoCalc keyboard version v%u.%u (0x%02x)\n", buf[1] >> 4, buf[1] & 0x0f, buf[1]);
        return;
    }
    printf("PicoCalc keyboard version unknown\n");
}

static int picocalc_i2c(const uint8_t msg, uint8_t *buf) {
    const int len_write = 1;
    int status = i2c_write_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, &msg, len_write, true, 500000);
    if (status != len_write) {
        return -1;
    }
    sleep_ms(16);
    const int len_read = 2;
    status = i2c_read_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, (uint8_t *)buf, len_read, false, 500000);
    if (status != len_read) {
        return -2;
    }
    return 0;
}