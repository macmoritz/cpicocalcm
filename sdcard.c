/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/*
 * initially copied from fatfs/source/diskio.c
 * adjusted for use in CPicoCalc/M project
 * Docstrings are copied from fatfs too.
 */

#include "sdcard.h"          /* Declarations FatFs MAI */
#include "fatfs/source/ff.h" /* Basic definitions of FatFs */
#include "picocalc.h"
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/timer.h>
#include <math.h>
#include <pico/aon_timer.h>
#include <pico/time.h>
#include <pico/types.h>
#include <pico/util/datetime.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEV_MMC 0 /* Map MMC/SD card to physical drive 0 */

/* State */
CARD_TYPE card_type = CARD_TYPE_UNKNOWN;
CSD card_csd;
bool card_initialized = false;

DSTATUS disk_status(BYTE pdrv) {
    if (!card_initialized) {
        return STA_NOINIT;
    }

    if (!picoCalcSDCardPresent || pdrv != DEV_MMC) {
        return STA_NODISK;
    }

    return RES_OK;
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != DEV_MMC) {
        return STA_NOINIT;
    }

    sleep_ms(1);
    gpio_init(SD_SPIO_CS);
    gpio_set_dir(SD_SPIO_CS, GPIO_OUT);
    gpio_put(SD_SPIO_CS, 1);

    gpio_init(SD_SPIO_TX);
    gpio_set_dir(SD_SPIO_TX, GPIO_OUT);
    gpio_put(SD_SPIO_TX, 1);

    gpio_init(SD_SPIO_RX);
    gpio_set_pulls(SD_SPIO_RX, true, false);

    spi_init(SD_PICO_SPI, 100 * 1000); // 100 kHz; clock rate needs to be between 100kHz and 400kHz
    gpio_set_function(SD_SPIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_SPIO_TX, GPIO_FUNC_SPI);
    gpio_set_function(SD_SPIO_RX, GPIO_FUNC_SPI);

    // Apply 74 or more clock pulses to SCLK
    wait_spi_clock_pulses(80);
    gpio_put(SD_SPIO_CS, 0);

    uint8_t r1;
    uint8_t r3[RESPONSE_R3];
    uint8_t r7[RESPONSE_R7];
    if (!send_command(CMD_0, 0, &r1)) {
        LOG("send_command CMD_0 STA_NOINIT\n");
        return STA_NOINIT;
    }
    if (!R1_IN_IDLE_STATE(r1)) {
        LOG("Unknown Card\n");
        return STA_NOINIT;
    }
    bool status = send_command(CMD_8, 0x1aa, r7);
    r1 = R3_R7_GET_R1(r7);
    uint32_t r7_32 = R3_R7_TO_32_BIT(r7);
    if (status && r7_32 == 0x1aa) {
        r1 = 0x1;
        while (R1_IN_IDLE_STATE(r1)) {
            if (!send_command(ACMD_41, 0x40000000, &r1)) {
                LOG("Unknown Card\n");
                return STA_NOINIT;
            }
            if (R1_SUCCESSFUL(r1)) {
                send_command(CMD_58, 0, r3);
                r1 = R3_R7_GET_R1(r3);
                if (CCS_BIT_IN_OCR(r3)) {
                    LOG("SD VER.2+ (Block address)\n");
                    card_type = CARD_TYPE_SD_VERSION_2_PLUS_BLOCK_ADDRESS;
                    break;
                }
                card_type = CARD_TYPE_SD_VERSION_2_PLUS_BYTE_ADDRESS;
                LOG("SD VER.2+ (Byte address)\n");
            }
        }
    } else if (status && R1_ILLEGAL_COMMAND(r1)) {
        r1 = 0x01;
        while (R1_IN_IDLE_STATE(r1)) {
            send_command(ACMD_41, 0, &r1);
            if (!R1_SUCCESSFUL(r1)) {
                r1 = 0x01;
                while (R1_IN_IDLE_STATE(r1)) {
                    send_command(CMD_1, 0, &r1);
                    if (!R1_SUCCESSFUL(r1)) {
                        LOG("Unknown Card\n");
                        return STA_NOINIT;
                    } else if (R1_SUCCESSFUL(r1)) {
                        card_type = CARD_TYPE_MMC_VERSION_3;
                        LOG("MMC Ver.3\n");
                        break;
                    }
                }
            } else {
                card_type = CARD_TYPE_SD_VERSION_1;
                LOG("SD Ver.1\n");
                break;
            }
        }
    }

    if (card_type == CARD_TYPE_UNKNOWN) {
        LOG("Unknown Card\n");
        return STA_NOINIT;
    }

    if (card_type == CARD_TYPE_SD_VERSION_2_PLUS_BYTE_ADDRESS || card_type == CARD_TYPE_SD_VERSION_1 || card_type == CARD_TYPE_MMC_VERSION_3) {
        // Set block size to 512 bytes
        send_command(CMD_16, BLOCK_SIZE, &r1);
    }

    // Read CSD and store globally
    status = send_command(CMD_9, 0, &r1);
    if (!status || !R1_SUCCESSFUL(r1)) {
        LOG("Could not read CSD register to determine maximum SPI speed.\n");
    }
    if (!get_data_packet(DATA_TOKEN_CMD_17_18_24, (unsigned char *)&card_csd, 16)) {
        LOG("Could not read CSD register to determine maximum SPI speed.\n");
    }

    status = set_maximum_spi_speed();
    if (!status) {
        return STA_NOINIT;
    }

    card_initialized = true;
    sd_spi_deselect();
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buffer, LBA_t sector, UINT count) {
    if (pdrv != DEV_MMC) {
        return RES_PARERR;
    }

    if (!picoCalcSDCardPresent || !card_initialized) {
        return RES_NOTRDY;
    }

    if (!sd_spi_select()) {
        return RES_ERROR;
    }

    const bool multiple_read = count > 1;
    const CMD cmd = multiple_read ? CMD_18 : CMD_17;
    const uint32_t address = (card_type == CARD_TYPE_SD_VERSION_2_PLUS_BYTE_ADDRESS) ? sector * BLOCK_SIZE : sector;
    uint8_t r1 = 0xff;
    send_command(cmd, address, &r1);
    if (!R1_SUCCESSFUL(r1)) {
        sd_spi_deselect();
        return RES_ERROR;
    }
    for (UINT received_count = 0; received_count != count; received_count++) {
        if (!get_data_packet(DATA_TOKEN_CMD_17_18_24, buffer + received_count * BLOCK_SIZE, BLOCK_SIZE)) {
            sd_spi_deselect();
            return RES_ERROR;
        }
    }
    if (multiple_read) {
        // CMD_18 needs to be cancelled by CMD_12
        send_command(CMD_12, 0, &r1);
        if (!R1_SUCCESSFUL(r1)) {
            sd_spi_deselect();
            return RES_ERROR;
        }
    }

    sd_spi_deselect();
    return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != DEV_MMC) {
        return RES_PARERR;
    }

    if (!picoCalcSDCardPresent || !card_initialized) {
        return RES_NOTRDY;
    }

    if (!sd_spi_select()) {
        return RES_ERROR;
    }

    const bool multiple_write = count > 1;
    const CMD cmd = multiple_write ? CMD_25 : CMD_24;
    const uint32_t address = (card_type == CARD_TYPE_SD_VERSION_2_PLUS_BYTE_ADDRESS) ? sector * BLOCK_SIZE : sector;
    uint8_t r1 = 0xff;
    bool status = send_command(cmd, address, &r1);
    if (!status || !R1_SUCCESSFUL(r1)) {
        sd_spi_deselect();
        return RES_ERROR;
    }

    for (UINT i = 0; i < count; i++) {
        bool status = send_data_packet(cmd == CMD_25 ? DATA_TOKEN_CMD_25 : DATA_TOKEN_CMD_17_18_24, buff + i * BLOCK_SIZE, BLOCK_SIZE);
        if (!status) {
            sd_spi_deselect();
            return RES_ERROR;
        }

        if (!wait_for_sd_ready()) {
            sd_spi_deselect();
            return RES_ERROR;
        }
    }

    if (multiple_write) {
        const uint8_t data[2] = {STOP_TOKEN_CMD_25, 0xff};
        spi_write_blocking(SD_PICO_SPI, (uint8_t *)&data, 2);
    }
    if (!wait_for_sd_ready()) {
        sd_spi_deselect();
        return RES_ERROR;
    }

    sd_spi_deselect();
    return RES_OK;
}

#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv != DEV_MMC) {
        return RES_PARERR;
    }

    if (!picoCalcSDCardPresent || !card_initialized) {
        return RES_NOTRDY;
    }

    switch (cmd) {
    case CTRL_SYNC:
        // Makes sure that the device has finished pending write process.
        // If the disk I/O layer or storage device has a write-back cache,
        // the dirty cached data must be committed to the medium immediately.
        // Nothing to do for this command if each write operation to the
        // medium is completed in the disk_write function.
        if (!wait_for_sd_ready()) {
            return RES_ERROR;
        }
        break;
    case GET_SECTOR_COUNT:
        // Retrieves number of available sectors (the largest allowable LBA + 1)
        // on the drive into the LBA_t variable that pointed by buff.
        // This command is used by f_mkfs and f_fdisk function to determine the
        // size of volume/partition to be created.
        const uint8_t csd_version = (card_csd[0] >> 6) & 0x01;
        if (csd_version == 0) {
            // CSD Version 1.0
            // READ_BL_LEN starts at 44, 4 bits width
            // C_SIZE starts at bit 54, 12 bits width
            // C_SIZE_MULT starts at bit 78, 3 bits width
            const uint read_bl_len = pow(2, card_csd[5] & 0x0f);
            const uint block_len = pow(2, read_bl_len);
            const uint c_size_mult = pow(2, ((card_csd[9] >> 6) & 0x7) + 2);
            const uint mult = pow(2, c_size_mult);
            const uint c_size = ((card_csd[6] & 0x03) << 10) | (card_csd[7] << 2) | ((card_csd[8] & 0xC0) >> 6);
            const uint blocknr = (c_size + 1) * mult;
            *(LBA_t *)buff = blocknr * block_len / BLOCK_SIZE; // TODO: are block_len and BLOCK_SIZE equal?
        } else if (csd_version == 1) {
            // CSD Version 2.0
            // C_SIZE starts at bit 58, 22 bits width
            const uint c_size = ((card_csd[7] << 16) | (card_csd[8] << 8) | card_csd[9]) & 0x3fffff;
            *(LBA_t *)buff = (c_size + 1) * 1024; // (c_size + 1) * 512 kB / BLOCK_SIZE(512 B) = (c_size + 1) * 1 kB = (c_size + 1) * 1024
        }
        break;
    case GET_BLOCK_SIZE:
        // Retrieves erase block size in unit of sector of the flash memory media
        // into the DWORD variable that pointed by buff. The allowable value is
        // 1 to 32768 in power of 2. Return 1 if it is unknown or in non-flash memory
        // media. This command is used by f_mkfs function with block size not specified
        // and it attempts to align the data area on the suggested block boundary. Note
        // that FatFs does not have an FTL (flash translation layer), so that either disk
        // I/O layer or storage device must have an FTL in it.

        // ERASE_BLK_EN is bit 81
        // SECTOR_SIZE starts at 82, 7 blocks width
        const uint8_t erase_blk_en = card_csd[10] & 0x40;
        const uint8_t sector_size = ((card_csd[10] & 0x3f) << 1) | ((card_csd[11] & 0x80) >> 7);
        *(DWORD *)buff = erase_blk_en ? 1 : (sector_size + 1);
        break;
    case CTRL_TRIM:
        // Informs the storage device that the data on the block of sectors is no longer
        // needed and it may or may not be erased. The block of sectors is specified in
        // an LBA_t array {<Start LBA>, <End LBA>} that pointed by buff. This is an identical
        // command to Trim in ATA devices. Nothing to do for this command if this funcion is
        // not supported or not a flash memory media. FatFs does not check the result code and
        // the file function is not affected even if the function failed. This command is called
        // on remove a cluster chain and in the f_mkfs function. It is required when FF_USE_TRIM == 1.
        return RES_OK;

    default:
        return RES_PARERR;
    }

    return RES_OK;
}

DWORD get_fattime(void) {
    struct tm tm;
    aon_timer_get_time_calendar(&tm);

    DWORD fattime = 0;
    fattime |= ((tm.tm_year - 80) & 0x7f) << 25; // tm.tm_year is years since 1900
    fattime |= ((tm.tm_mon + 1) & 0xf) << 21;    // tm.tm_mon is 0..11
    fattime |= (tm.tm_mday & 0x1f) << 16;
    fattime |= (tm.tm_hour & 0x1f) << 11;
    fattime |= (tm.tm_min & 0x3f) << 5;
    fattime |= (tm.tm_sec >> 1) & 0x1f;

    return fattime;
}

struct timespec get_seconds_since_epoch_from_fattime(WORD date, WORD time) {
    struct tm datetime = {
        .tm_year = 80 + ((date & 0x7f) >> 25),
        .tm_mon = ((date & 0xf) >> 21) - 1,
        .tm_mday = ((date & 0x1f) >> 16),
        .tm_hour = ((time & 0x1f) >> 11),
        .tm_min = ((time & 0x3f) >> 5),
        .tm_sec = (time & 0x1f) * 2,
        .tm_isdst = 0,
    };
    time_t t = pico_mktime(&datetime);
    struct timespec ts = {
        .tv_sec = t,
        .tv_nsec = 0,
    };
    return ts;
}

int sd_spi_tx_dma;
int sd_spi_rx_dma;

static inline bool sd_spi_select() {
    gpio_put(SD_SPIO_CS, 0);
    uint8_t dst;
    spi_read_blocking(SD_PICO_SPI, 0xff, &dst, 1);

    if (!wait_for_sd_ready()) {
        sd_spi_deselect();
        return false;
    }
    return true;
}

static inline void sd_spi_deselect() {
    gpio_put(SD_SPIO_CS, 1);
    uint8_t dst;
    spi_read_blocking(SD_PICO_SPI, 0xff, &dst, 1);
}

static inline bool send_command(CMD cmd, uint32_t argument, uint8_t *response) {
    const size_t tx_buffer_size = 6; // CMD + argument + CRC
    uint8_t tx_buffer[tx_buffer_size];

    // CMD
    if (cmd > 63) {
        send_command(CMD_55, 0, response);
        tx_buffer[0] = 0x40 | (cmd - 63);
    } else {
        tx_buffer[0] = 0x40 | cmd;
    }

    // Argument
    tx_buffer[1] = (argument >> 24) & 0xff;
    tx_buffer[2] = (argument >> 16) & 0xff;
    tx_buffer[3] = (argument >> 8) & 0xff;
    tx_buffer[4] = argument & 0xff;

    // CRC, only needed in initialization for CMD_1 and CMD_8
    if (cmd == CMD_0) {
        tx_buffer[tx_buffer_size - 1] = 0x95;
    } else if (cmd == CMD_8) {
        tx_buffer[tx_buffer_size - 1] = 0x87;
    } else {
        // after successful initialization no CRC is needed
        tx_buffer[tx_buffer_size - 1] = 0x01;
    }

    for (size_t i = 0; i < tx_buffer_size; i++) {
        spi_write_blocking(SD_PICO_SPI, &tx_buffer[i], 1);
    }

    if (cmd == CMD_12) {
        // read stuff byte
        uint8_t dummy;
        spi_read_blocking(SD_PICO_SPI, 0xff, &dummy, 1);
    }

    uint8_t r1 = 0xff;
    absolute_time_t deadline = make_timeout_time_ms(500);
    while (!time_reached(deadline)) {
        spi_read_blocking(SD_PICO_SPI, 0xff, &r1, 1);
        if (r1 != 0xff) {
            break;
        }
    }
    if (r1 == 0xff) {
        return false;
    }
    response[0] = r1;

    if (cmd == CMD_12) {
        // R1b = R1 + busy
        absolute_time_t deadline = make_timeout_time_ms(500);
        while (!time_reached(deadline)) {
            spi_read_blocking(SD_PICO_SPI, 0xff, &r1, 1);
            if (r1 == 0xff) {
                break;
            }
        }
    }

    if (cmd == CMD_8 || cmd == CMD_58) {
        // R3 = R7 = R1 + trailing 32-bit data
        spi_read_blocking(SD_PICO_SPI, 0xff, &response[1], 4);
    }

    return true;
}

static inline void wait_spi_clock_pulses(size_t clockPulses) {
    const uint8_t tx_buffer = 0xff;

    for (size_t i = 0; i < clockPulses; i += 8) {
        spi_write_blocking(SD_PICO_SPI, &tx_buffer, 1);
    }
}

static inline bool get_data_packet(uint8_t data_token, BYTE *buffer, size_t count) {
    absolute_time_t deadline = make_timeout_time_ms(500);
    uint8_t r1 = 0xff;
    while (!time_reached(deadline)) {
        spi_read_blocking(SD_PICO_SPI, 0xff, &r1, 1);
        if (r1 == data_token) {
            break;
        }
    }
    if (r1 != data_token) {
        return false;
    }
    spi_read_blocking(SD_PICO_SPI, 0xff, buffer, count);
    uint8_t crc[2] = {0, 0};
    spi_read_blocking(SD_PICO_SPI, 0xff, (uint8_t *)&crc, 2);
    return true;
}

static inline bool send_data_packet(const uint8_t data_token, const BYTE *buffer, size_t count) {
    const uint8_t data[3] = {data_token, 0, 0};
    spi_write_blocking(SD_PICO_SPI, (uint8_t *)&data, 1);
    spi_write_blocking(SD_PICO_SPI, buffer, count);
    spi_write_blocking(SD_PICO_SPI, &data[1], 2);

    absolute_time_t deadline = make_timeout_time_ms(500);
    uint8_t data_response = 0xff;
    while (!time_reached(deadline)) {
        spi_read_blocking(SD_PICO_SPI, 0xff, &data_response, 1);
        if (DATA_RESPONSE_ACCEPTED(data_response)) {
            return true;
        }
    }
    return false;
}

static inline bool wait_for_sd_ready() {
    uint8_t r1 = 0;
    absolute_time_t deadline = make_timeout_time_ms(500);
    while (!time_reached(deadline)) {
        spi_read_blocking(SD_PICO_SPI, 0xff, &r1, 1);
        if (r1 == 0xff) {
            return true;
        }
    }
    return false;
}

static inline bool set_maximum_spi_speed() {
    uint spi_baudrate = 5 * 1e6; // 5MHz fallback speed
    const uint8_t tran_speed = card_csd[3];
    if (tran_speed != 0x00) {
        const uint transfer_rate_unit = pow(10, (5 + (tran_speed & 0x3)));
        const float time_value_constants[16] = {0, 1.0, 1.2, 1.3, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 7.0, 8.0};
        const float time_value = time_value_constants[(tran_speed >> 3) & 0xf];
        spi_baudrate = transfer_rate_unit * time_value;
    }

    const uint speed = spi_set_baudrate(SD_PICO_SPI, spi_baudrate);
    LOG("Set SPI speed to %dHz, requested %dHz\n", speed, spi_baudrate);
    return speed <= spi_baudrate;
}
