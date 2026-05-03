// initial version from https://github.com/clockworkpi/PicoCalc/tree/master/Code/picocalc_kbd_tester/lcdspi
#ifndef LCDSPI_H
#define LCDSPI_H
#include <hardware/spi.h>
#include <stdint.h>

// #define LCD_SPI_SPEED 25000000
#define LCD_SPI_SPEED SYS_CLK_HZ

#define PICO_LCD_SCK 10 // Clock
#define PICO_LCD_TX 11  // MOSI
#define PICO_LCD_RX 12  // MISO
#define PICO_LCD_CS 13  // Chip Select
#define PICO_LCD_DC 14
#define PICO_LCD_RST 15

// Pico spi0 or spi1 must match GPIO pins used above.
#define PICO_LCD_SPI_MOD spi1

#define LCD_WIDTH 320
#define LCD_HEIGHT 320

#define PIXFMT_BGR 1

#define TFT_SLPOUT 0x11
#define TFT_INVOFF 0x20
#define TFT_INVON 0x21

#define TFT_DISPOFF 0x28
#define TFT_DISPON 0x29
#define TFT_MADCTL 0x36

#define ILI9488_MEMCONTROL 0x36
#define ILI9488_MADCTL_MX 0x40
#define ILI9488_MADCTL_BGR 0x08

#define ILI9488_COLADDRSET 0x2A
#define ILI9488_PAGEADDRSET 0x2B
#define ILI9488_MEMORYWRITE 0x2C
#define ILI9488_RAMRD 0x2E

#define ILI9488_Portrait ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR

#define ORIENT_NORMAL 0

#define COLOR_TYPE uint16_t
// #define RGB(red, green, blue) (unsigned int)(((red & 0b11111111) << 16) | ((green & 0b11111111) << 8) | (blue & 0b11111111))
#define RGB565(red, green, blue) (COLOR_TYPE)(((red & 0x1f) << 11) | ((green & 0x3f) << 5) | ((blue & 0x1f)))
#define BLACK RGB565(0, 0, 0)
#define BLUE RGB565(0, 0, 31)
#define RED RGB565(31, 0, 0)
#define MAGENTA RGB565(31, 0, 31)
#define GREEN RGB565(0, 63, 0)
#define CYAN RGB565(0, 63, 31)
#define YELLOW RGB565(31, 63, 0)
#define WHITE RGB565(31, 63, 31)

#define PORTCLR 1
#define PORTSET 2
#define PORTINV 3
#define LAT 4
#define LATCLR 5
#define LATSET 6
#define LATINV 7
#define ODC 8
#define ODCCLR 9
#define ODCSET 10
#define CNPU 12
#define CNPUCLR 13
#define CNPUSET 14
#define CNPUINV 15
#define CNPD 16
#define CNPDCLR 17
#define CNPDSET 18

#define ANSELCLR -7
#define ANSELSET -6
#define ANSELINV -5
#define TRIS -4
#define TRISCLR -3
#define TRISSET -2

extern void __not_in_flash_func(spi_write_fast)(spi_inst_t *spi, const uint8_t *src, size_t len);
extern void __not_in_flash_func(spi_finish)(spi_inst_t *spi);
extern void hw_read_spi(unsigned char *buff, int cnt);
extern void hw_send_spi(const unsigned char *buff, int cnt);
extern unsigned char __not_in_flash_func(hw1_swap_spi)(unsigned char data_out);

extern void lcd_spi_raise_cs(void);
extern void lcd_spi_lower_cs(void);
extern void spi_write_data(unsigned char data);
extern void spi_write_command(unsigned char data);
extern void spi_write_cd(unsigned char command, int data, ...);
extern void spi_write_data24(uint32_t data);
extern void define_region_spi(int xstart, int ystart, int xend, int yend, int rw);

/**
 * @brief Initializes the communication with the lcd and the lcd itself.
 * Needs to be called before calling any other `lcd_`-prefixed function.
 *
 */
extern void lcd_init();
/**
 * @brief Clear the lcd screen by drawing a screen-filling black rect.
 *
 */
extern void lcd_clear();
/**
 * @brief Resets the lcd.
 * Important for reading the lcd memory.
 */
extern void lcd_reset_controller(void);
extern void pin_set_bit(int pin, unsigned int offset);

/**
 * @brief Show a character at a specified location on the lcd screen.
 * Not printable ascii characters will be printed as a block in the background color.
 *
 * @param c Character to show
 * @param fc Foreground color
 * @param bc Background color
 * @param x Vertical coordinate
 * @param y Horizontal coordinate
 */
extern void lcd_print_char(char c, COLOR_TYPE fc, COLOR_TYPE bc, int x, int y, bool underline);
/**
 * @brief Draw a filled rectangle
 *
 * @param x1 Top-left vertical coordinate
 * @param y1 Top-left horizontal coordinate
 * @param x2 Bottom-right vertical coordinate
 * @param y2 Bottom-right horizontal coordinate
 * @param c Fill color
 */
extern void lcd_draw_rect(int x1, int y1, int x2, int y2, COLOR_TYPE c);
/**
 * @brief Print the bitmap of a char on the video output
 *
 * @param x1 Top-left vertical position of the bitmap
 * @param y1 Top-left horizontal position of the bitmap
 * @param width width of the bitmap
 * @param height height of the bitmap
 * @param scale positive scaling of the char horizontally and vertically
 * @param fc foreground color
 * @param bc background color
 * @param bitmap pointer to the bitmap
 */
extern void lcd_draw_bitmap(int x1, int y1, int width, int height, COLOR_TYPE fc, COLOR_TYPE bc, const unsigned char *bitmap);

static inline const void color_to_buffer(uint16_t color, unsigned char *buffer);

#endif