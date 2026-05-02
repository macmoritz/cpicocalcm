// initial version from https://github.com/clockworkpi/PicoCalc/tree/master/Code/picocalc_kbd_tester/lcdspi
#include <hardware/platform_defs.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/timer.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "font.h"
#include "lcd.h"

static short hres = 0;
static short vres = 0;
unsigned char lcd_buffer[320 * 3] = {0}; // 1440 = 480*3, 320*3 = 960

void __not_in_flash_func(spi_write_fast)(spi_inst_t *spi, const uint8_t *src, size_t len) {
    // Write to TX FIFO whilst ignoring RX, then clean up afterward. When RX
    // is full, PL022 inhibits RX pushes, and sets a sticky flag on
    // push-on-full, but continues shifting. Safe if SSPIMSC_RORIM is not set.
    for (size_t i = 0; i < len; ++i) {
        while (!spi_is_writable(spi))
            tight_loop_contents();
        spi_get_hw(spi)->dr = (uint32_t)src[i];
    }
}

void __not_in_flash_func(spi_finish)(spi_inst_t *spi) {
    // Drain RX FIFO, then wait for shifting to finish (which may be *after*
    // TX FIFO drains), then drain RX FIFO again
    while (spi_is_readable(spi))
        (void)spi_get_hw(spi)->dr;
    while (spi_get_hw(spi)->sr & SPI_SSPSR_BSY_BITS)
        tight_loop_contents();
    while (spi_is_readable(spi))
        (void)spi_get_hw(spi)->dr;

    // Don't leave overrun flag set
    spi_get_hw(spi)->icr = SPI_SSPICR_RORIC_BITS;
}

void define_region_spi(int xstart, int ystart, int xend, int yend, int rw) {
    unsigned char coord[4];
    lcd_spi_lower_cs();
    gpio_put(PICO_LCD_DC, 0); // gpio_put(PICO_LCD_DC,0);
    hw_send_spi(&(uint8_t){ILI9488_COLADDRSET}, 1);
    gpio_put(PICO_LCD_DC, 1);
    coord[0] = xstart >> 8;
    coord[1] = xstart;
    coord[2] = xend >> 8;
    coord[3] = xend;
    hw_send_spi(coord, 4); //		HAL_SPI_Transmit(&hspi3,coord,4,500);
    gpio_put(PICO_LCD_DC, 0);
    hw_send_spi(&(uint8_t){ILI9488_PAGEADDRSET}, 1);
    gpio_put(PICO_LCD_DC, 1);
    coord[0] = ystart >> 8;
    coord[1] = ystart;
    coord[2] = yend >> 8;
    coord[3] = yend;
    hw_send_spi(coord, 4); //		HAL_SPI_Transmit(&hspi3,coord,4,500);
    gpio_put(PICO_LCD_DC, 0);
    if (rw) {
        hw_send_spi(&(uint8_t){ILI9488_MEMORYWRITE}, 1);
    } else {
        hw_send_spi(&(uint8_t){ILI9488_RAMRD}, 1);
    }
    gpio_put(PICO_LCD_DC, 1);
}

void lcd_draw_bitmap(int x1, int y1, int width, int height, int scale, int fc, int bc, const unsigned char *bitmap) {
    int i, j, k, m, n;
    char f[3], b[3];
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    char *p = 0;
    union colourmap {
        char rgbbytes[4];
        unsigned int rgb;
    } c;

    if (x1 >= hres || y1 >= vres || x1 + width * scale < 0 || y1 + height * scale < 0)
        return;
    // adjust when part of the bitmap is outside the displayable coordinates
    vertCoord = y1;
    if (y1 < 0)
        y1 = 0; // the y coord is above the top of the screen
    XStart = x1;
    if (XStart < 0)
        XStart = 0; // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1;
    if (XEnd >= hres)
        XEnd = hres - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1;
    if (YEnd >= vres)
        YEnd = vres - 1; // the height of the bitmap will extend beyond the bottom margin

    // convert the colours to 565 format
    f[0] = (fc >> 16);
    f[1] = (fc >> 8) & 0xFF;
    f[2] = (fc & 0xFF);
    b[0] = (bc >> 16);
    b[1] = (bc >> 8) & 0xFF;
    b[2] = (bc & 0xFF);

    define_region_spi(XStart, y1, XEnd, YEnd, 1);

    n = 0;
    for (i = 0; i < height; i++) {    // step thru the font scan line by line
        for (j = 0; j < scale; j++) { // repeat lines to scale the font
            if (vertCoord++ < 0)
                continue;           // we are above the top of the screen
            if (vertCoord > vres) { // we have extended beyond the bottom of the screen
                lcd_spi_raise_cs(); // set CS high
                return;
            }
            horizCoord = x1;
            for (k = 0; k < width; k++) {     // step through each bit in a scan line
                for (m = 0; m < scale; m++) { // repeat pixels to scale in the x axis
                    if (horizCoord++ < 0)
                        continue; // we have not reached the left margin
                    if (horizCoord > hres)
                        continue; // we are beyond the right margin
                    if ((bitmap[((i * width) + k) / 8] >> (((height * width) - ((i * width) + k) - 1) % 8)) & 1) {
                        hw_send_spi((uint8_t *)&f, 3);
                    } else {
                        if (bc == -1) {
                            // TODO: cleanup?
                            c.rgbbytes[0] = p[n];
                            c.rgbbytes[1] = p[n + 1];
                            c.rgbbytes[2] = p[n + 2];
                            b[0] = c.rgbbytes[2];
                            b[1] = c.rgbbytes[1];
                            b[2] = c.rgbbytes[0];
                        }
                        hw_send_spi((uint8_t *)&b, 3);
                    }
                    n += 3;
                }
            }
        }
    }
    lcd_spi_raise_cs(); // set CS high
}

void lcd_draw_rect(int x1, int y1, int x2, int y2, int c) {
    // convert the color to 565 format
    unsigned char col[3];
    if (x1 == x2 && y1 == y2) {
        if (x1 < 0)
            return;
        if (x1 >= hres)
            return;
        if (y1 < 0)
            return;
        if (y1 >= vres)
            return;
        define_region_spi(x1, y1, x2, y2, 1);
        col[0] = (c >> 16);
        col[1] = (c >> 8) & 0xFF;
        col[2] = (c & 0xFF);
        hw_send_spi(col, 3);
    } else {
        int i, t, y;
        unsigned char *p;
        // make sure the coordinates are kept within the display area
        if (x2 <= x1) {
            t = x1;
            x1 = x2;
            x2 = t;
        }
        if (y2 <= y1) {
            t = y1;
            y1 = y2;
            y2 = t;
        }
        if (x1 < 0)
            x1 = 0;
        if (x1 >= hres)
            x1 = hres - 1;
        if (x2 < 0)
            x2 = 0;
        if (x2 >= hres)
            x2 = hres - 1;
        if (y1 < 0)
            y1 = 0;
        if (y1 >= vres)
            y1 = vres - 1;
        if (y2 < 0)
            y2 = 0;
        if (y2 >= vres)
            y2 = vres - 1;
        define_region_spi(x1, y1, x2, y2, 1);
        i = x2 - x1 + 1;
        i *= 3;
        p = lcd_buffer;
        col[0] = (c >> 16);
        col[1] = (c >> 8) & 0xFF;
        col[2] = (c & 0xFF);
        for (t = 0; t < i; t += 3) {
            p[t] = col[0];
            p[t + 1] = col[1];
            p[t + 2] = col[2];
        }
        for (y = y1; y <= y2; y++) {
            spi_write_fast(PICO_LCD_SPI_MOD, p, i);
        }
    }
    spi_finish(PICO_LCD_SPI_MOD);
    lcd_spi_raise_cs();
}

void lcd_print_char(char c, int fc, int bc, int x, int y, bool underline) {
    const unsigned char *char_buf;
    int scale = 0x01;

    if (c < font_metadata.ascii_code_min || c > font_metadata.ascii_code_max) {
        lcd_draw_rect(x, y, x + (font_metadata.char_width * scale), y + (font_metadata.char_height * scale), bc);

        return;
    }

    char_buf = font + (int)(((c - font_metadata.ascii_code_min) * font_metadata.char_height * font_metadata.char_width) / 8);
    lcd_draw_bitmap(x, y, font_metadata.char_width, font_metadata.char_height, scale, fc, bc, char_buf);
    if (underline) {
        const int ypos = y + font_metadata.char_height - 1;
        lcd_draw_rect(x, ypos, x + font_metadata.char_width - 1, ypos, fc);
    }
}

void lcd_clear() {
    lcd_draw_rect(0, 0, hres - 1, vres - 1, BLACK);
}

void hw_read_spi(unsigned char *buff, int cnt) {
    spi_read_blocking(PICO_LCD_SPI_MOD, 0xff, buff, cnt);
}

void hw_send_spi(const unsigned char *buff, int cnt) {
    spi_write_blocking(PICO_LCD_SPI_MOD, buff, cnt);
}

void pin_set_bit(int pin, unsigned int offset) {
    switch (offset) {
    case LATCLR:
        gpio_set_pulls(pin, false, false);
        gpio_pull_down(pin);
        gpio_put(pin, 0);
        return;
    case LATSET:
        gpio_set_pulls(pin, false, false);
        gpio_pull_up(pin);
        gpio_put(pin, 1);
        return;
    case LATINV:
        gpio_xor_mask(1 << pin);
        return;
    case TRISSET:
        gpio_set_dir(pin, GPIO_IN);
        sleep_us(2);
        return;
    case TRISCLR:
        gpio_set_dir(pin, GPIO_OUT);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
        sleep_us(2);
        return;
    case CNPUSET:
        gpio_set_pulls(pin, true, false);
        return;
    case CNPDSET:
        gpio_set_pulls(pin, false, true);
        return;
    case CNPUCLR:
    case CNPDCLR:
        gpio_set_pulls(pin, false, false);
        return;
    case ODCCLR:
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
        sleep_us(2);
        return;
    case ODCSET:
        gpio_set_pulls(pin, true, false);
        gpio_set_dir(pin, GPIO_IN);
        sleep_us(2);
        return;
    case ANSELCLR:
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, GPIO_IN);
        return;
    default:
        break;
        // printf("Unknown pin_set_bit command");
    }
}

void lcd_reset_controller(void) {
    pin_set_bit(PICO_LCD_RST, LATSET);
    sleep_us(10000);
    pin_set_bit(PICO_LCD_RST, LATCLR);
    sleep_us(10000);
    pin_set_bit(PICO_LCD_RST, LATSET);
    sleep_us(200000);
}

void lcd_spi_raise_cs(void) {
    gpio_put(PICO_LCD_CS, 1);
}

void lcd_spi_lower_cs(void) {
    gpio_put(PICO_LCD_CS, 0);
}

void spi_write_data(unsigned char data) {
    gpio_put(PICO_LCD_DC, 1);
    lcd_spi_lower_cs();
    hw_send_spi(&data, 1);
    lcd_spi_raise_cs();
}

void spi_write_data24(uint32_t data) {
    uint8_t data_array[3];
    data_array[0] = data >> 16;
    data_array[1] = (data >> 8) & 0xFF;
    data_array[2] = data & 0xFF;

    gpio_put(PICO_LCD_DC, 1); // Data mode
    gpio_put(PICO_LCD_CS, 0);
    spi_write_blocking(PICO_LCD_SPI_MOD, data_array, 3);
    gpio_put(PICO_LCD_CS, 1);
}

void spi_write_command(unsigned char data) {
    gpio_put(PICO_LCD_DC, 0);
    gpio_put(PICO_LCD_CS, 0);

    spi_write_blocking(PICO_LCD_SPI_MOD, &data, 1);

    gpio_put(PICO_LCD_CS, 1);
}

void spi_write_cd(unsigned char command, int data, ...) {
    int i;
    va_list ap;
    va_start(ap, data);
    spi_write_command(command);
    for (i = 0; i < data; i++) {
        spi_write_data((char)va_arg(ap, int));
    }
    va_end(ap);
}

void lcd_init() {
    // init GPIO
    gpio_init(PICO_LCD_SCK);
    gpio_init(PICO_LCD_TX);
    gpio_init(PICO_LCD_RX);
    gpio_init(PICO_LCD_CS);
    gpio_init(PICO_LCD_DC);
    gpio_init(PICO_LCD_RST);

    gpio_set_dir(PICO_LCD_SCK, GPIO_OUT);
    gpio_set_dir(PICO_LCD_TX, GPIO_OUT);
    // gpio_set_dir(PICO_LCD_RX, GPIO_IN);
    gpio_set_dir(PICO_LCD_CS, GPIO_OUT);
    gpio_set_dir(PICO_LCD_DC, GPIO_OUT);
    gpio_set_dir(PICO_LCD_RST, GPIO_OUT);

    // init SPI
    const int baudrate = spi_init(PICO_LCD_SPI_MOD, LCD_SPI_SPEED);
    gpio_set_function(PICO_LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PICO_LCD_TX, GPIO_FUNC_SPI);
    gpio_set_function(PICO_LCD_RX, GPIO_FUNC_SPI);
    gpio_set_input_hysteresis_enabled(PICO_LCD_RX, true);

    printf("SPI baudrate: %d\n", baudrate);

    gpio_put(PICO_LCD_CS, 1);
    gpio_put(PICO_LCD_RST, 1);

    lcd_reset_controller();

    hres = 320;
    vres = 320;

    spi_write_command(0xE0); // Positive Gamma Control
    spi_write_data(0x00);
    spi_write_data(0x03);
    spi_write_data(0x09);
    spi_write_data(0x08);
    spi_write_data(0x16);
    spi_write_data(0x0A);
    spi_write_data(0x3F);
    spi_write_data(0x78);
    spi_write_data(0x4C);
    spi_write_data(0x09);
    spi_write_data(0x0A);
    spi_write_data(0x08);
    spi_write_data(0x16);
    spi_write_data(0x1A);
    spi_write_data(0x0F);

    spi_write_command(0XE1); // Negative Gamma Control
    spi_write_data(0x00);
    spi_write_data(0x16);
    spi_write_data(0x19);
    spi_write_data(0x03);
    spi_write_data(0x0F);
    spi_write_data(0x05);
    spi_write_data(0x32);
    spi_write_data(0x45);
    spi_write_data(0x46);
    spi_write_data(0x04);
    spi_write_data(0x0E);
    spi_write_data(0x0D);
    spi_write_data(0x35);
    spi_write_data(0x37);
    spi_write_data(0x0F);

    spi_write_command(0XC0); // Power Control 1
    spi_write_data(0x17);
    spi_write_data(0x15);

    spi_write_command(0xC1); // Power Control 2
    spi_write_data(0x41);

    spi_write_command(0xC5); // VCOM Control
    spi_write_data(0x00);
    spi_write_data(0x12);
    spi_write_data(0x80);

    spi_write_command(TFT_MADCTL); // Memory Access Control
    spi_write_data(0x48);          // MX, BGR

    spi_write_command(0x3A); // Pixel Interface Format
    spi_write_data(0x66);    // 18 bit colour for SPI

    spi_write_command(0xB0); // Interface Mode Control
    spi_write_data(0x00);

    spi_write_command(0xB1); // Frame Rate Control
    spi_write_data(0xA0);

    spi_write_command(TFT_INVON);

    spi_write_command(0xB4); // Display Inversion Control
    spi_write_data(0x02);

    spi_write_command(0xB6); // Display Function Control
    spi_write_data(0x02);
    spi_write_data(0x02);
    spi_write_data(0x3B);

    spi_write_command(0xB7); // Entry Mode Set
    spi_write_data(0xC6);
    spi_write_command(0xE9);
    spi_write_data(0x00);

    spi_write_command(0xF7); // Adjust Control 3
    spi_write_data(0xA9);
    spi_write_data(0x51);
    spi_write_data(0x2C);
    spi_write_data(0x82);

    spi_write_command(TFT_SLPOUT); // Exit Sleep
    sleep_ms(120);

    spi_write_command(TFT_DISPON); // Display on
    sleep_ms(120);

    spi_write_command(TFT_MADCTL);
    spi_write_cd(ILI9488_MEMCONTROL, 1, ILI9488_Portrait);
}