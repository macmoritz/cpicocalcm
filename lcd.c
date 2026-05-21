// initial version from https://github.com/clockworkpi/PicoCalc/tree/master/Code/picocalc_kbd_tester/lcdspi
#include <hardware/platform_defs.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/timer.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
#include <sys/_types.h>
#include <wchar.h>

#include "font.h"
#include "lcd.h"
#include "ncurses.h"

static short hres = 0;
static short vres = 0;
uint spi_tx_dma;
dma_channel_config spi_tx_dma_cfg;

static inline void lcd_define_region16(int xstart, int ystart, int xend, int yend) {
    uint16_t coord[2];

    coord[0] = (uint16_t)xstart;
    coord[1] = (uint16_t)xend;
    spi_write_command16(ILI9488_COLADDRSET, coord, 2);

    coord[0] = (uint16_t)ystart;
    coord[1] = (uint16_t)yend;
    spi_write_command16(ILI9488_PAGEADDRSET, coord, 2);
}

static inline void lcd_draw_rect(int x1, int y1, int w, int h, COLOR_TYPE color) {
    // make sure the coordinates are kept within the display area
    int x2 = x1 + w - 1;
    int y2 = y1 + h - 1;
    const uint32_t pixels = w * h;
    if (x1 < 0) {
        x1 = 0;
    }
    if (x1 >= hres) {
        x1 = hres - 1;
    }
    if (x2 < 0) {
        x2 = 0;
    }
    if (x2 >= hres) {
        x2 = hres - 1;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (y1 >= vres) {
        y1 = vres - 1;
    }
    if (y2 < 0) {
        y2 = 0;
    }
    if (y2 >= vres) {
        y2 = vres - 1;
    }
    lcd_define_region16(x1, y1, x2, y2);

    spi_write_command16(ILI9488_MEMORYWRITE, NULL, 0);
    channel_config_set_read_increment(&spi_tx_dma_cfg, false);
    dma_channel_configure(
        spi_tx_dma,
        &spi_tx_dma_cfg,
        &spi_get_hw(PICO_LCD_SPI_MOD)->dr,
        &color,
        pixels,
        true);

    dma_channel_wait_for_finish_blocking(spi_tx_dma);

    while (spi_is_busy(PICO_LCD_SPI_MOD)) {
        tight_loop_contents();
    }

    channel_config_set_read_increment(&spi_tx_dma_cfg, true);
    dma_channel_set_config(spi_tx_dma, &spi_tx_dma_cfg, false);
}

static inline void render_line(WINDOW *screen, int line, COLOR_TYPE *buffer) {
    const cchar_t *content;
    const unsigned char *glyph;
    COLOR_TYPE fc, bc;

    for (int col = 0; col < screen->cols; col++) {
        size_t baseX = col * font_metadata.char_width;
        content = &screen->content[line * screen->cols + col];
        glyph = font + (int)((content->content - font_metadata.ascii_code_min) * font_metadata.char_height);
        fc = RGB_COLOR[content->foreground];
        bc = RGB_COLOR[content->background];

        const bool isValidChar = content->content >= font_metadata.ascii_code_min && content->content <= font_metadata.ascii_code_max;
        const bool isCursorVisible = screen->cursorVisibility != invisible && screen->curx == col && screen->cury == line;
        if (!isValidChar || (screen->blinkstate && (content->blink || isCursorVisible))) {
            const COLOR_TYPE color = isCursorVisible ? fc : bc;
            for (int y = 0; y < font_metadata.char_height; y++) {
                for (int x = 0; x < font_metadata.char_width; x++) {
                    const size_t dst = y * LCD_WIDTH + baseX + x;
                    buffer[dst] = color;
                }
            }
            continue;
        }

        for (int y = 0; y < font_metadata.char_height; y++) {
            uint8_t glyphRow = glyph[y];
            if (content->bold) {
                glyphRow |= (glyphRow >> 1);
            }

            for (int x = 0; x < font_metadata.char_width; x++) {
                const size_t dst = y * LCD_WIDTH + baseX + x;
                bool pixelSet = (glyphRow & (0x80 >> x)) != 0;
                buffer[dst] = pixelSet ? fc : bc;
            }
        }

        if (content->underline) {
            size_t y = font_metadata.char_height - 1;
            size_t row = y * LCD_WIDTH;
            for (size_t xi = 0; xi < font_metadata.char_width; xi++) {
                buffer[row + baseX + xi] = fc;
            }
        }
    }
}

// TODO: try to use two chained DMA buffers and wait at the end
void lcd_update(WINDOW *screen) {
    const size_t bufferSize = LCD_WIDTH * font_metadata.char_height;

    COLOR_TYPE *buffer;
    COLOR_TYPE *buffer0 = malloc(bufferSize * sizeof(COLOR_TYPE));
    COLOR_TYPE *buffer1 = malloc(bufferSize * sizeof(COLOR_TYPE));
    if (!buffer0 || !buffer1) {
        free(buffer0);
        free(buffer1);
        return;
    }

    dma_channel_set_write_addr(spi_tx_dma, &spi_get_hw(PICO_LCD_SPI_MOD)->dr, false);
    lcd_define_region16(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    spi_write_command16(ILI9488_MEMORYWRITE, NULL, 0);

    for (int line = 0; line < screen->lines; line++) {
        buffer = (line & 1) ? buffer0 : buffer1;
        render_line(screen, line, buffer);

        if (line > 0) {
            dma_channel_wait_for_finish_blocking(spi_tx_dma);
            while (spi_is_busy(PICO_LCD_SPI_MOD)) {
                tight_loop_contents();
            }
        }

        dma_channel_set_read_addr(spi_tx_dma, buffer, false);
        dma_channel_set_trans_count(spi_tx_dma, bufferSize, true);
    }

    free(buffer0);
    free(buffer1);
}

void lcd_clear() {
    lcd_draw_rect(0, 0, hres, vres, BLACK);
}

void lcd_reset_controller(void) {
    gpio_set_pulls(PICO_LCD_RST, true, false);
    gpio_put(PICO_LCD_RST, 1);
    sleep_us(10000);
    gpio_set_pulls(PICO_LCD_RST, false, true);
    gpio_put(PICO_LCD_RST, 0);
    sleep_us(10000);
    gpio_set_pulls(PICO_LCD_RST, true, false);
    gpio_put(PICO_LCD_RST, 1);
    sleep_us(200000);
}

void spi_write_command(const uint8_t command, const uint8_t *args, size_t len_args) {
    gpio_put(PICO_LCD_DC, 0);
    spi_write_blocking(PICO_LCD_SPI_MOD, &command, 1);
    gpio_put(PICO_LCD_DC, 1);

    if (args != NULL) {
        gpio_put(PICO_LCD_DC, 1);
        spi_write_blocking(PICO_LCD_SPI_MOD, args, len_args);
    }
}

void spi_write_command16(uint8_t command, const uint16_t *args, size_t len_args) {
    // 0x00 interpreted as a command is a NO-OP.
    const uint16_t cmd16 = (uint16_t)command;

    gpio_put(PICO_LCD_DC, 0);
    spi_write16_blocking(PICO_LCD_SPI_MOD, &cmd16, 1);
    gpio_put(PICO_LCD_DC, 1);

    if (args != NULL) {
        spi_write16_blocking(PICO_LCD_SPI_MOD, args, len_args);
    }
}

void lcd_init() {
    // init GPIO
    gpio_init(PICO_LCD_CS);
    gpio_init(PICO_LCD_DC);
    gpio_init(PICO_LCD_RST);

    gpio_set_dir(PICO_LCD_CS, GPIO_OUT);
    gpio_set_dir(PICO_LCD_DC, GPIO_OUT);
    gpio_set_dir(PICO_LCD_RST, GPIO_OUT);

    // init SPI
    const int baudrate = spi_init(PICO_LCD_SPI_MOD, LCD_SPI_SPEED);
    printf("SPI baudrate: %d\n", baudrate);
    spi_set_format(PICO_LCD_SPI_MOD, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PICO_LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PICO_LCD_TX, GPIO_FUNC_SPI);
    // gpio_set_function(PICO_LCD_RX, GPIO_FUNC_SPI);

    spi_get_hw(PICO_LCD_SPI_MOD)->dmacr = SPI_SSPDMACR_TXDMAE_BITS;
    spi_tx_dma = dma_claim_unused_channel(true);
    spi_tx_dma_cfg = dma_channel_get_default_config(spi_tx_dma);
    channel_config_set_transfer_data_size(&spi_tx_dma_cfg, DMA_SIZE_16);
    channel_config_set_dreq(&spi_tx_dma_cfg, spi_get_dreq(PICO_LCD_SPI_MOD, true));
    channel_config_set_write_increment(&spi_tx_dma_cfg, false);
    channel_config_set_read_increment(&spi_tx_dma_cfg, true);
    dma_channel_set_config(spi_tx_dma, &spi_tx_dma_cfg, false);

    gpio_put(PICO_LCD_CS, 1);
    gpio_put(PICO_LCD_RST, 1);

    lcd_reset_controller();

    hres = 320;
    vres = 320;

    gpio_put(PICO_LCD_CS, 0);
    gpio_put(PICO_LCD_DC, 1);

    uint8_t init[15];
    // Positive Gamma Control
    init[0] = 0x00;
    init[1] = 0x03;
    init[2] = 0x09;
    init[3] = 0x08;
    init[4] = 0x16;
    init[5] = 0x0A;
    init[6] = 0x3F;
    init[7] = 0x78;
    init[8] = 0x4C;
    init[9] = 0x09;
    init[10] = 0x0A;
    init[11] = 0x08;
    init[12] = 0x16;
    init[13] = 0x1A;
    init[14] = 0x0F;
    spi_write_command(0xE0, init, 15);

    // Negative Gamma Control
    init[0] = 0x00;
    init[1] = 0x16;
    init[2] = 0x19;
    init[3] = 0x03;
    init[4] = 0x0F;
    init[5] = 0x05;
    init[6] = 0x32;
    init[7] = 0x45;
    init[8] = 0x46;
    init[9] = 0x04;
    init[10] = 0x0E;
    init[11] = 0x0D;
    init[12] = 0x35;
    init[13] = 0x37;
    init[14] = 0x0F;
    spi_write_command(0xE1, init, 15);

    // Power Control 1
    init[0] = 0x17;
    init[1] = 0x15;
    spi_write_command(0xC0, init, 2);

    // Power Control 2
    init[0] = 0x41;
    spi_write_command(0xC1, init, 1);

    // VCOM Control
    init[0] = 0x00;
    init[1] = 0x12;
    init[2] = 0x80;
    spi_write_command(0xC5, init, 3);

    // Pixel Interface Format
    init[0] = 0x55; // 16 bit color for SPI
    spi_write_command(0x3A, init, 1);

    // Interface Mode Control
    init[0] = 0x00; // SPI_EN = “0”, DIN and DOUT pins are used for 3/4 wire serial interface.
    spi_write_command(0xB0, init, 1);

    // Frame Rate Control
    init[0] = 0xA0;
    spi_write_command(0xB1, init, 1);

    spi_write_command(TFT_INVON, NULL, 0);

    // Display Inversion Control
    init[0] = 0x02;
    spi_write_command(0xB4, init, 1);

    // Display Function Control
    init[0] = 0x02;
    init[1] = 0x02;
    init[2] = 0x3B;
    spi_write_command(0xB6, init, 3);

    // Entry Mode Set
    init[0] = 0xC6;
    init[1] = 0xE9;
    init[2] = 0x00;
    spi_write_command(0xB7, init, 3);

    // Adjust Control 3
    init[0] = 0xA9;
    init[1] = 0x51;
    init[2] = 0x2C;
    init[3] = 0x82;
    spi_write_command(0xF7, init, 4);

    // Exit Sleep
    spi_write_command(TFT_SLPOUT, NULL, 0);
    sleep_ms(120);

    // Display on
    spi_write_command(TFT_DISPON, NULL, 0);
    sleep_ms(120);

    // Memory Access Control
    init[0] = ILI9488_Portrait;
    spi_write_command(TFT_MADCTL, init, 1);

    spi_set_format(PICO_LCD_SPI_MOD, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}
