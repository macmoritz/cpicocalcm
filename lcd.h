// initial version from https://github.com/clockworkpi/PicoCalc/tree/master/Code/picocalc_kbd_tester/lcdspi
#ifndef LCDSPI_H
#define LCDSPI_H
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <stdint.h>

#define LCD_SPI_SPEED SYS_CLK_HZ // set SPI speed to maximum, microcontroller selects the highest available frequency

#define PICO_LCD_SCK 10 // Clock
#define PICO_LCD_TX 11  // MOSI
#define PICO_LCD_RX 12  // MISO
#define PICO_LCD_CS 13  // Chip Select
#define PICO_LCD_DC 14  // Data Command Selection
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
#define RGB565(red, green, blue) (COLOR_TYPE)(((red & 0x1f) << 11) | ((green & 0x3f) << 5) | ((blue & 0x1f)))
#define BLACK RGB565(0, 0, 0)
#define BLUE RGB565(0, 0, 31)
#define RED RGB565(31, 0, 0)
#define MAGENTA RGB565(31, 0, 31)
#define GREEN RGB565(0, 63, 0)
#define CYAN RGB565(0, 63, 31)
#define YELLOW RGB565(31, 63, 0)
#define WHITE RGB565(31, 63, 31)

extern uint spi_tx_dma;
extern dma_channel_config spi_tx_dma_cfg;

extern const size_t bufferSize;
extern COLOR_TYPE *buffer0;
extern COLOR_TYPE *buffer1;

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
 *
 */
extern void lcd_reset_controller(void);

/**
 * @brief Draw a filled rectangle
 *
 * @param x1 Top-left vertical coordinate
 * @param y1 Top-left horizontal coordinate
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 * @param c Fill color
 */
static inline void lcd_draw_rect(int x1, int y1, int w, int h, COLOR_TYPE c);

/**
 * @brief Sends information to the lcd controller, so it knows where to place the following pixel data
 *
 * @param xstart Top-left vertical coordinate
 * @param ystart Top-left horizontal coordinate
 * @param xend Bottom-right vertical coordinate
 * @param yend Bottom-right horizontal coordinate
 */
static inline void lcd_define_region16(int xstart, int ystart, int xend, int yend);

/**
 * @brief Sends a command to the lcd controller with optional arguments
 *
 * @param command 8-Bit command, gets expanded to 16-Bit internally
 * @param args Optional arguments to pass after the command
 * @param len_args Count of arguments
 */
static inline void spi_write_command16(uint8_t command, const uint16_t *args, size_t len_args);

typedef struct _win_st WINDOW; // Forward declaration for ncurses.h

/**
 * @brief Renders the screen content and pushes it to the lcd
 * Runs blocking, but does double buffering to achieve fast SPI writing
 *
 * @param screenPointer Pointer to the ncurses `WINDOW` type containing screen content information
 */
void lcd_update(WINDOW *screenPointer);

/**
 * @brief Renders the nth-line from the screen to the buffer
 *
 * @param screen Pointer to the ncurses `WINDOW` type containing screen content information
 * @param line Index of line to render
 * @param buffer Pointer to a buffer for the rendered pixel data
 */
static void render_line(WINDOW *screen, int line, COLOR_TYPE *buffer);

#endif