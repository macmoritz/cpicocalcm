#include "picocalc.h"
#include <pico/types.h>
#include <stdint.h>

#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>

void picocalc_init() {
    gpio_init(AUDIO_PWM_L);
    gpio_init(AUDIO_PWM_R);
    gpio_set_function(AUDIO_PWM_L, GPIO_FUNC_PWM);
    gpio_set_function(AUDIO_PWM_R, GPIO_FUNC_PWM);
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
