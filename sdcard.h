/*-----------------------------------------------------------------------/
/  Low level disk interface module include file   (C)ChaN, 2025          /
/-----------------------------------------------------------------------*/
/*
    initially copied from fatfs/source/diskio.h
    adjusted for use in CPicoCalc/M project
*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#include "fatfs/source/ff.h"
#include <hardware/dma.h>
#include <stdint.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Status of Disk Functions */
typedef BYTE DSTATUS;

/* Results of Disk Functions */
typedef enum {
    RES_OK = 0, /* 0: Successful */
    RES_ERROR,  /* 1: R/W Error */
    RES_WRPRT,  /* 2: Write Protected */
    RES_NOTRDY, /* 3: Not Ready */
    RES_PARERR  /* 4: Invalid Parameter */
} DRESULT;

/**
 * @brief SPI Command Set
 * Value is 0 to 63 (6 bit).
 * ACMD<n> means a command sequense of CMD55-CMD<n>. They have the value (63 + n).
 */
typedef enum {
    /**
     * GO_IDLE_STATE: Software reset
     *
     * Argument: None
     *
     * Response: R1
     *
     * Data: None
     */
    CMD_0 = 0,
    /**
     * SEND_OP_COND: Initiate initialization process
     *
     * Argument: None
     *
     * Response: R1
     *
     * Data: None
     */
    CMD_1 = 1,
    /**
     * APP_SEND_OP_COND: For only SDC. Initiate initialization process.
     *
     * Argument: Rsv(0)[31], HCS[30], Rsv(0)[29:0]
     *
     * Response: R1
     *
     * Data: None
     */
    ACMD_41 = (63 + 41),
    /**
     * SEND_IF_COND: For only SDC V2. Check voltage range.
     *
     * Argument: Rsv(0)[31:12], Supply Voltage(1)[11:8], Check Pattern(0xAA)[7:0]
     *
     * Response: R7
     *
     * Data: None
     */
    CMD_8 = 8,
    /**
     * SEND_CSD: Read CSD register.
     *
     * Argument: None
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_9 = 9,
    /**
     * SEND_CID: Read CID register.
     *
     * Argument: None
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_10 = 10,
    /**
     * STOP_TRANSMISSION: Stop to read data.
     *
     * Argument: None
     *
     * Response: R1b
     *
     * Data: None
     */
    CMD_12 = 12,
    /**
     * SET_BLOCKLEN: Change R/W block size.
     *
     * Argument: Block length[31:0]
     *
     * Response: R1
     *
     * Data: None
     */
    CMD_16 = 16,
    /**
     * READ_SINGLE_BLOCK: Read a block.
     *
     * Argument: Address[31:0]
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_17 = 17,
    /**
     * READ_MULTIPLE_BLOCK: Read multiple blocks.
     * Reads continuously, needs to be stopped by sending `CMD_12`.
     *
     * Argument: Address[31:0]
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_18 = 18,
    /**
     * SET_BLOCK_COUNT: For only MMC. Define number of blocks to transfer with next multi-block read/write command.
     *
     * Argument: Number of blocks[15:0]
     *
     * Response: R1
     *
     * Data: None
     */
    CMD_23 = 23,
    /**
     * SET_WR_BLOCK_ERASE_COUNT: For only SDC. Define number of blocks to pre-erase with next multi-block write command.
     *
     * Argument: Number of blocks[22:0]
     *
     * Response: R1
     *
     * Data: None
     */
    ACMD_23 = (63 + 23),
    /**
     * WRITE_BLOCK: Write a block.
     *
     * Argument: Address[31:0]
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_24 = 24,
    /**
     * WRITE_MULTIPLE_BLOCK: Write multiple blocks.
     *
     * Argument: Address[31:0]
     *
     * Response: R1
     *
     * Data: Yes
     */
    CMD_25 = 25,
    /**
     * APP_CMD: Leading command of ACMD<n> command.
     *
     * Argument: None
     *
     * Response: R1
     *
     * Data: None
     */
    CMD_55 = 55,
    /**
     * READ_OCR: Read OCR.
     *
     * Argument: None
     *
     * Response: R3
     *
     * Data: None
     */
    CMD_58 = 58,
} CMD;

typedef enum {
    RESPONSE_R1 = 1,
    RESPONSE_R3 = 5,
    RESPONSE_R7 = 5,
} RESPONSE;

typedef uint8_t CSD[16]; // 128 bit

#define R1_IN_IDLE_STATE(R1) (R1 & 0x1)
#define R1_ERASE_RESET(R1) (R1 & 0x2)
#define R1_ILLEGAL_COMMAND(R1) (R1 & 0x4)
#define R1_COMMAND_CRC_ERROR(R1) (R1 & 0x8)
#define R1_ERASE_SEQUENCE_ERROR(R1) (R1 & 0x10)
#define R1_ADDRESS_ERROR(R1) (R1 & 0x20)
#define R1_PARAMETER_ERROR(R1) (R1 & 0x40)
#define R1_SUCCESSFUL(R1) (R1 == 0x00)
#define R3_R7_GET_R1(R) (R[0])
#define R3_R7_TO_32_BIT(R3_R7) (((uint32_t)R3_R7[1] << 24) | ((uint32_t)R3_R7[2] << 16) | ((uint32_t)R3_R7[3] << 8) | ((uint32_t)R3_R7[4] << 0))
#define CCS_BIT_IN_OCR(R3) (R3_R7_TO_32_BIT(R3) & (1u << 30)) // 30th bit

#define LOG(...) printf("SD FatFs: " __VA_ARGS__)
// clang-format off
// #define LOG(...) {} while(0)
// clang-format on

#define DATA_TOKEN_CMD_17_18_24 0xfe
#define DATA_TOKEN_CMD_25 0xfc
#define STOP_TOKEN_CMD_25 0xfd

#define DATA_RESPONSE_ACCEPTED(DR) ((DR & 0x1f) == 0x05)
#define DATA_RESPONSE_CRC_ERROR(DR) ((DR & 0x1f) == 0x0b)
#define DATA_RESPONSE_WRITE_ERROR(DR) ((DR & 0x1f) == 0x0d)

#define BLOCK_SIZE 512

typedef enum {
    CARD_TYPE_UNKNOWN = 0,
    CARD_TYPE_MMC_VERSION_3,
    CARD_TYPE_SD_VERSION_1,
    CARD_TYPE_SD_VERSION_2_PLUS_BYTE_ADDRESS,
    CARD_TYPE_SD_VERSION_2_PLUS_BLOCK_ADDRESS,
} CARD_TYPE;

/* State */
extern CARD_TYPE card_type;
extern CSD card_csd;
extern bool card_initialized;

/* SPI Helpers */
extern int sd_spi_tx_dma;
extern int sd_spi_rx_dma;

static inline bool sd_spi_select();

static inline void sd_spi_deselect();

/**
 * @brief Set the maximum spi speed allowed by the SD Card.
 * From https://academy.cba.mit.edu/classes/networking_communications/SD/SD.pdf:
 * TRAN_SPEED defines the maximum data transfer rate per one data line
 * | Bits | Code |
 * |------|------|
 * | 2:0  | transfer rate unit 0=100kbit/s, 1=1Mbit/s, 2=10Mbit/s, 3=100Mbit/s, 4...7=reserved |
 * | 6:3  | time value 0=reserved, 1=1.0, 2=1.2, 3=1.3, 4=1.5, 5=2.0, 6=2.5, 7=3.0, 8=3.5, 9=4.0, A=4.5, B=5.0, C=5.5, D=6.0, E=7.0, F=8.0 |
 * | 7    | reserved |
 *
 * @return true Got CSD value and set speed successfully.
 * @return false No CSD value from SD Card or SPI speed is higher than requested.
 */
static inline bool set_maximum_spi_speed();

static inline bool send_command(CMD cmd, uint32_t argument, uint8_t *response);

static inline void wait_spi_clock_pulses(size_t clockPulses);

static inline bool get_data_packet(uint8_t data_token, BYTE *buffer, size_t count);

static inline bool send_data_packet(const uint8_t data_token, const BYTE *buffer, size_t count);

/**
 * @brief Wait for data from sd card. Maximum wait time is 500ms.
 *
 * @return true Got data which was not 0xff.
 * @return false Got only 0xff data.
 */
static inline bool wait_for_sd_ready();

/**
 * @brief The disk_initialize function is called to initializes the storage device.
 *
 * @param pdrv Physical drive number, needs to be 0 (DEV_MMC)
 * @return DSTATUS This function returns the current drive status flags as the result. For details of the drive status, refer to the `disk_status` function.
 */
DSTATUS disk_initialize(BYTE pdrv);

/**
 * @brief The disk_status function is called to inquire the current drive status.
 *
 * @param pdrv Physical drive number, needs to be 0 (DEV_MMC)
 * @return DSTATUS The current drive status is returned in combination of status flags described below. FatFs refers only STA_NOINIT and STA_PROTECT.
 * STA_NOINIT: Indicates that the device has not been initialized and not ready to work. This flag is set on system reset, media removal or failure of disk_initialize function. It is cleared on disk_initialize function succeeded. Any media change that occurs asynchronously must be captured and reflect it to the status flags, or auto-mount function will not work correctly. If the system does not support media change detection, application program needs to explicitly re-mount the volume with f_mount function after each media change.
 * STA_NODISK: Indicates that no medium in the drive. This is always cleared when the drive is non-removable class. Note that FatFs does not refer this flag.
 * STA_PROTECT: Indicates that the medium is write protected. This is always cleared when the drive has no write protect function. Not valid if STA_NODISK is set.
 */
DSTATUS disk_status(BYTE pdrv);

/**
 * @brief The disk_read function is called to read data from the storage device.
 *
 * @param pdrv Physical drive number, needs to be 0 (DEV_MMC)
 * @param buff Pointer to the first item of the byte array to store read data. Size of read data will be the sector size * count bytes.
 * @param sector Start sector number in LBA. The data type LBA_t is an alias of DWORD or QWORD depends on the configuration option.
 * @param count Number of sectors to read
 * @return DRESULT
 * - RES_OK (0): The function succeeded.
 * - RES_ERROR: An unrecoverable hard error occured during the read operation.
 * - RES_PARERR: Invalid parameter.
 * - RES_NOTRDY: The device has not been initialized.
 */
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);

/**
 * @brief The disk_write function is called to write data to the storage device.
 *
 * @param pdrv Physical drive number, needs to be 0 (DEV_MMC)
 * @param buff Pointer to the first item of the byte array to be written. The size of data to be written is sector size * count bytes.
 * @param sector Start sector in LBA
 * @param count Number of sectors to write
 * @return DRESULT
 * - RES_OK (0): The function succeeded.
 * - RES_ERROR: An unrecoverable hard error occured during the write operation.
 * - RES_WRPRT: The medium is write protected.
 * - RES_PARERR: Invalid parameter.
 * - RES_NOTRDY: The device has not been initialized.
 */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);

/**
 * @brief The disk_ioctl function is called to control device specific features and miscellaneous functions other than generic read/write.
 *
 * @param pdrv Physical drive number, needs to be 0 (DEV_MMC)
 * @param cmd Command code
 * - GET_SECTOR_SIZE is not implemented, since FF_MAX_SS == FF_MIN_SS. FatFs Docs: "This command is required only when FF_MAX_SS > FF_MIN_SS. When FF_MAX_SS == FF_MIN_SS, it will never be used and the disk_read and disk_write function must work in FF_MAX_SS bytes/sector."
 *
 * @param buff Pointer to the parameter depends on the command code. Do not care if the command has no parameter to be passed.
 * @return DRESULT
 * - RES_OK (0): The function succeeded.
 * - RES_ERROR: An unrecoverable hard error occured during the read operation.
 * - RES_PARERR: Invalid parameter.
 * - RES_NOTRDY: The device has not been initialized.
 */

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

/**
 * @brief The get_fattime function is called to get the current time.
 *
 * @return DWORD Current local time shall be returned as bit-fields packed into a DWORD value. The bit fields are as follows:
 *  - bit31:25 - 7 bits - Year origin from the 1980 (0..127, e.g. 37 for 2017)
 *  - bit24:21 - 4 bits - Month (1..12)
 *  - bit20:16 - 5 bits - Day of the month (1..31)
 *  - bit15:11 - 5 bits - Hour (0..23)
 *  - bit10:5  - 6 bits - Minute (0..59)
 *  - bit4:0   - 5 bits - Second / 2 (0..29, e.g. 25 for 50)
 */
DWORD get_fattime(void);

struct timespec get_seconds_since_epoch_from_fattime(WORD date, WORD time);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT 0x01  /* Drive not initialized */
#define STA_NODISK 0x02  /* No medium in the drive */
#define STA_PROTECT 0x04 /* Write protected */

/* Command code for disk_ioctrl fucntion */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC 0        /* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT 1 /* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE 2  /* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE 3   /* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM 4        /* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER 5  /* Get/Set power status */
#define CTRL_LOCK 6   /* Lock/Unlock media removal */
#define CTRL_EJECT 7  /* Eject media */
#define CTRL_FORMAT 8 /* Create physical format on the media */

/* MMC/SDC specific ioctl command (Not used by FatFs) */
#define MMC_GET_TYPE 10   /* Get card type */
#define MMC_GET_CSD 11    /* Get CSD */
#define MMC_GET_CID 12    /* Get CID */
#define MMC_GET_OCR 13    /* Get OCR */
#define MMC_GET_SDSTAT 14 /* Get SD status */
#define ISDIO_READ 55     /* Read data form SD iSDIO register */
#define ISDIO_WRITE 56    /* Write data to SD iSDIO register */
#define ISDIO_MRITE 57    /* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command (Not used by FatFs) */
#define ATA_GET_REV 20   /* Get F/W revision */
#define ATA_GET_MODEL 21 /* Get model name */
#define ATA_GET_SN 22    /* Get serial number */

#ifdef __cplusplus
}
#endif

#endif
