#pragma once

#include <stdint.h>

// https://github.com/clockworkpi/PicoCalc/blob/master/clockwork_Mainboard_V2.0_Schematic.pdf
#define AUDIO_PWM_L 26
#define AUDIO_PWM_R 27

/*
 * Initializes communication with picocalc.
 * Needs to be called before calling any other `picocalc_` function.
 */
void picocalc_init();

/*
 * Produces a static tone with given parameters. Runs blocking.
 * param freq: Frequency of the tone in Hz
 * param duration: Duration of the tone in milliseconds
 */
void picocalc_beep(uint32_t freq, uint32_t duration);