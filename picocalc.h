#pragma once

#include <hardware/i2c.h>
#include <stdint.h>

// https://github.com/clockworkpi/PicoCalc/blob/master/Code/picocalc_kbd_tester/i2ckbd/i2ckbd.h
#define I2C_KBD_MOD i2c1
#define I2C_KBD_SDA 6
#define I2C_KBD_SCL 7
#define I2C_KBD_SPEED 10000 // if dual i2c, then the speed of keyboard i2c should be 10khz
#define I2C_KBD_ADDR 0x1F

// https://github.com/clockworkpi/PicoCalc/blob/master/clockwork_Mainboard_V2.0_Schematic.pdf
#define AUDIO_PWM_L 26
#define AUDIO_PWM_R 27

/**
 * @brief Initializes communication with PicoCalc.
 * @note Needs to be called before calling any other `picocalc_` function.
 */
void picocalc_init();

/**
 * @brief Drains the PicoCalc keyboard fifo.
 *
 * @return -1 if error, any other value is returned from picocalc fifo count
 */
int picocalc_drain_keyboard_fifo();

/**
 * @brief Produces a static tone with given parameters. Runs blocking.
 *
 * @param freq Frequency of the tone in Hz
 * @param duration Duration of the tone in milliseconds
 */
void picocalc_beep(uint32_t freq, uint32_t duration);

/**
 * @brief Reads blocking from the PicoCalc keyboard via i2c.
 * @note implementation based on https://github.com/clockworkpi/PicoCalc/blob/master/Code/picocalc_kbd_tester/i2ckbd/i2ckbd.c
 *
 * @return Keycode, or -1 if something went wrong
 */
int picocalc_read_kbd();

/**
 * @brief Reads the battery level from PicoCalc blocking.
 * @note implementation based on https://github.com/clockworkpi/PicoCalc/blob/master/Code/picocalc_kbd_tester/i2ckbd/i2ckbd.c
 *
 * @return percent level of the battery, or -1 if something went wrong
 */
int picocalc_read_battery();

/**
 * @brief Prints the version reported by the PicoCalc keyboard (STM32).
 */
void picocalc_print_version();

/**
 * @brief Gets data from PicoCalc keyboard (STM32).
 * @param function
 *  - 0x01: fw version
 *  - 0x02: config
 *  - 0x03: interrupt status
 *  - 0x04: key status
 *  - 0x05: backlight
 *  - 0x06: debounce cfg
 *  - 0x07: poll freq cfg
 *  - 0x08: reset
 *  - 0x09: fifo
 *  - 0x0A: keyboard backlight
 *  - 0x0b: battery
 *  - 0x0c: read c64 matrix
 *  - 0x0d: joystick io bits
 *  - 0x0e: POWER OFF
 * @param buffer Data sent by coprocessor.
 *
 * @return status, -1 is write error, -2 is read error, 0 is ok
 */
static int picocalc_i2c(const uint8_t function, uint8_t *buffer);