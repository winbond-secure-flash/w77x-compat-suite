/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       SPI_ops.c
* @brief      This file includes qlib to nxp bridge definitions for SPI on NXP 10xx device
*
* ### project W77Q
*
************************************************************************************************************/

#include "SPI_ops.h"
#if !defined(QLIB_NO_DIRECT_FLASH_ACCESS)
#include "qlib.h"
#endif
#include "defs.h"
#include "fsl_flexspi.h"
#include "fsl_debug_console.h"
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE   SIZE_256B_ // maximum write size
#endif


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NXP_105x_MAX_SDR_FREQ 332
#define NXP_105x_MAX_DTR_FREQ 166
#ifndef SPI_FLASH_CMD__READ_STATUS_REGISTER_1
// in Q3 READ_STATUS_REGISTER_1 is replaced by READ_STATUS_REGISTER
#define SPI_FLASH_CMD__READ_STATUS_REGISTER_1 SPI_FLASH_CMD__READ_STATUS_REGISTER
#endif


//#define FLASH_PAGE_SIZE 256
#define SECTOR_SIZE 0x1000 /* 4K */

#define NOR_CMD_LUT_SEQ_IDX_CODE_FETCH_CMD 0
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD 1
#define NOR_CMD_LUT_SEQ_IDX_READ_STATUS 2
#define NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE 3
#define NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR 4
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD 5
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP 6
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE 7
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL 8
#define NOR_CMD_LUT_SEQ_IDX_READID 9
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG 10
#define NOR_CMD_LUT_SEQ_IDX_ENTERQPI 11
#define NOR_CMD_LUT_SEQ_IDX_EXITQPI 12
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG 13
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST 14

#define CUSTOM_LUT_LENGTH 64
#define FLASH_QUAD_ENABLE 0x40
#define FLASH_BUSY_STATUS_POL 1
#define FLASH_BUSY_STATUS_OFFSET 0

// FlexSPI defines
#define FLASH_SIZE_IN_KB                          (FLASH_SIZE / 1024)
#define MAX_ADDRESS_RANGE_IN_KB                   (4 * 1024)
#define SPI_BUS_FREQ                              (133 * 1000 * 1000)
#define SPI_PLL_FREQ_480MHz                       (480)

#define FLEXSPI_MCR0_SERCLKDIV_MASK              (0x700U)
#define FLEXSPI_MCR0_SERCLKDIV_SHIFT             (8U)
//! SERCLKDIV - Serial Clock Div (Divide FLEXSPI_ROOT_CLK)
//*  0b000..Divide by 1
//*  0b111..Divide by 8

#define FLEXSPI_MCR0_SERCLKDIV(x)                (((uint32_t)(((uint32_t)(x)) << FLEXSPI_MCR0_SERCLKDIV_SHIFT)) & FLEXSPI_MCR0_SERCLKDIV_MASK)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
__attribute__ ((section(".ramfunc.$RAM")))
void GetFlexSpiFreqFracPodfDiv_L(uint32_t* freq, uint8_t* frac, uint8_t* podf, uint8_t* div, bool dtrMode, uint8_t* semc_podf, uint8_t* pllType);
/*******************************************************************************
 * Variables
 *****************************************************************************/
SPI_FREQUENCY_T g_spi_freq = 4; // Initial value 4MHz
SPI_FREQUENCY_T g_set_spi_freq = 4;
bool            g_set_dqs_mode = false;
bool            g_dtr_mode = false;
bool            g_dqs_mode = false;
bool            g_dqs_pol = false;
bool            g_spi_cs_select = false; // false=CS#1 , true=CS#2
float           g_tclqv = 4.5;
float           g_tdssq = 0.4;
uint8_t g_SequenceTable[256] = { 0 };

bool g_LUT_DTR = false; // whether LUT includes DOPI or SOPI commands (relevant only for 8_8_8 mode as LUT for other modes include both STR and DTR)
QLIB_BUS_MODE_T g_LUT_format = QLIB_BUS_MODE_INVALID;
#define IS_LUT_INITIALIZED (QLIB_BUS_MODE_INVALID != g_LUT_format)

typedef struct
{
    uint8_t frac; // x (18 / frac)
    uint8_t podf; // (podf + 1)
} flexspi_clk_freq_list_t;

flexspi_device_config_t deviceConfig =
{
    .flexspiRootClk = SPI_BUS_FREQ,
    .flashSize = (MAX_ADDRESS_RANGE_IN_KB * 1024),
    .CSIntervalUnit = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval = 2,
    .CSHoldTime = 3,
    .CSSetupTime = 3,
    .dataValidTime = 0,
    .columnspace = 0,
    .enableWordAddress = 0,
    .AWRSeqIndex = 0,
    .AWRSeqNumber = 0,
    .ARDSeqIndex = NOR_CMD_LUT_SEQ_IDX_CODE_FETCH_CMD,
    .ARDSeqNumber = 1,
    .AHBWriteWaitUnit = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};


static uint32_t customLUT[CUSTOM_LUT_LENGTH] = {

    FETCH_COMMAND_LUT,
    /* Normal read mode -SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x03, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Fast read mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x0B, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x08, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Fast read quad mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xEB, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x06, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    /* Read extend parameters */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x81, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Write Enable */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Erase Sector  */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xD7, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),

    /* Page Program - single mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x02, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Page Program - quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x32, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read ID */
    [4 * NOR_CMD_LUT_SEQ_IDX_READID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x9F, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enable Quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x01, kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enter QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_ENTERQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x35, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Exit QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_EXITQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xF5, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read status register */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Erase whole chip */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASECHIP] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xC7, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
};

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
const uint32_t W77Q_SINGLE_SDR_DTR_LUT[CUSTOM_LUT_LENGTH] =
{
        // OP0 Single SDR
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP0] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,       kFLEXSPI_1PAD, SEC_CMD_OP0_SDR_SINGLE,
                            kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SEC_CMD_OP0_SDR_DUMMY_CYCLES),
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP0 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR,  kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // OP0 Single DTR
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP0] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,       kFLEXSPI_1PAD, SEC_CMD_OP0_DTR_SINGLE,
                            kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_1PAD, 2 * SEC_CMD_OP0_DTR_DUMMY_CYCLES),
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP0 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR,  kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // OP1 in Single
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,       kFLEXSPI_1PAD, SEC_CMD_OP1_SDR_SINGLE,
                            kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SEC_CMD_OP1_CTAG_SIZE),

        // OP1 in Single with data
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1_Data] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,       kFLEXSPI_1PAD, SEC_CMD_OP1_SDR_SINGLE,
                            kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SEC_CMD_OP1_CTAG_SIZE),
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1_Data + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // OP2 in Single SDR
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP2] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,  kFLEXSPI_1PAD, SEC_CMD_OP2_SDR_SINGLE,
                            kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SEC_CMD_OP2_DUMMY_CYCLES),
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP2 + 1 ] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR,  kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // OP2 in Single DTR
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP2] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,  kFLEXSPI_1PAD, SEC_CMD_OP2_DTR_SINGLE,
                            kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_1PAD, 2 * SEC_CMD_OP2_DUMMY_CYCLES),
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP2 + 1 ] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR,  kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // Read Status Register
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_READ_STATUS_REG1] =
                FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPI_FLASH_CMD__READ_STATUS_REGISTER_1,
                                kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0),

        // Exit QPI (enter SPI in QPI mode)
        [4 * W77Q_LUT_SEQ_IDX_4_4_4_ENTER_SPI] =
                FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, SPI_FLASH_CMD__ENTER_SPI,
                                kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        // Enter QPI from SPI
        [4 * W77Q_LUT_SEQ_IDX_1_1_1_ENTER_QPI] =
                FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,  kFLEXSPI_1PAD, SPI_FLASH_CMD__ENTER_QPI,
                                kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

        [4 * W77Q_LUT_SEQ_IDX_TEMP] = 0
};
#endif
/*******************************************************************************
 * Local Functions
 ******************************************************************************/
#ifdef SPI_LUT_OPTIMIZATION_ENABLED
void flexspi_secure_LUT_update_L(QLIB_BUS_MODE_T format, bool dtrMode);
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
status_t flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write enable */
    flashXfer.deviceAddress = baseAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

status_t flexspi_nor_wait_bus_busy(FLEXSPI_Type *base)
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue;
    status_t status;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READSTATUSREG;
    flashXfer.data          = &readValue;
    flashXfer.dataSize      = 1;

    do
    {
        status = FLEXSPI_TransferBlocking(base, &flashXfer);

        if (status != kStatus_Success)
        {
            return status;
        }
        if (FLASH_BUSY_STATUS_POL)
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);

    return status;
}

status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base)
{
    flexspi_transfer_t flashXfer;
    status_t status;
    uint32_t writeValue = FLASH_QUAD_ENABLE;

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable quad mode. */
    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG;
    flashXfer.data          = &writeValue;
    flashXfer.dataSize      = 1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

    return status;
}

status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    flashXfer.deviceAddress = address;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress = address;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR;
    status                  = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

    return status;
}

status_t flexspi_nor_flash_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src, uint32_t length)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    status = flexspi_nor_write_enable(base, dstAddr);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress = dstAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD;
    flashXfer.data          = (uint32_t *)src;
    flashXfer.dataSize      = length;
    status                  = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT)
    base->AHBCR |= FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK;
    base->AHBCR &= ~(FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK);
#else
    FLEXSPI_SoftwareReset(base);
#endif

    return status;
}

status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    status = flexspi_nor_write_enable(base, dstAddr);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress = dstAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD;
    flashXfer.data          = (uint32_t *)src;
    flashXfer.dataSize      = FLASH_PAGE_SIZE;
    status                  = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT)
    base->AHBCR |= FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK;
    base->AHBCR &= ~(FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK);
#else
    FLEXSPI_SoftwareReset(base);
#endif

    return status;
}

status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId)
{
    uint32_t temp;
    flexspi_transfer_t flashXfer;
    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READID;
    flashXfer.data          = &temp;
    flashXfer.dataSize      = 1;

    status_t status = FLEXSPI_TransferBlocking(base, &flashXfer);

    *vendorId = temp;

    /* Do software reset. */
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT)
    base->AHBCR |= FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK;
    base->AHBCR &= ~(FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK);
#else
    FLEXSPI_SoftwareReset(base);
#endif

    return status;
}

status_t flexspi_nor_erase_chip(FLEXSPI_Type *base)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_ERASECHIP;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    return status;
}


#define PLL3_PFD0 3 // CSCMR1[FLEXSPI_SEL=3]=PLL3_PFD0
#define SEMC_ROOT_CLK 0 // CSCMR1[FLEXSPI_SEL=0]=SEMC_ROOT_CLK
#define PLL2_PFD2_CLK 2 // CSCMR1[PLL2_PFD2_CLK=0]=PLL2_PFD2_CLK
#define FLEX_SPI_FREQS_COUNT_ARRAY 165 // 165 individual parameters for frequencies between 1-327MHz
typedef struct
{
    uint32_t freq : 10;
    uint32_t frac: 8;
    uint32_t podf: 4;
    uint32_t div: 4;
    uint32_t semc_podf: 4;
    uint32_t pllType: 2;
} flexspi_param_freq_list_t;


static flexspi_param_freq_list_t FlexSpiFreqPodfFracDivArray[FLEX_SPI_FREQS_COUNT_ARRAY]= {
{1, 0, 8, 8, 8, SEMC_ROOT_CLK},
{2, 0, 7, 7, 6, SEMC_ROOT_CLK},
{3, 0, 8, 5, 5, SEMC_ROOT_CLK},
{4, 35, 8, 7, 0, PLL3_PFD0},
{5, 30, 8, 7, 0, PLL3_PFD0},
{6, 30, 8, 6, 0, PLL3_PFD0},
{7, 22, 8, 7, 0, PLL3_PFD0},
{8, 27, 8, 5, 0, PLL3_PFD0},
{9, 20, 8, 6, 0, PLL3_PFD0},
{10, 18, 8, 6, 0, PLL3_PFD0},
{11, 14, 8, 7, 0, PLL3_PFD0},
{12, 15, 8, 6, 0, PLL3_PFD0},
{13, 22, 6, 5, 0, PLL3_PFD0},
{14, 22, 7, 4, 0, PLL3_PFD0},
{15, 18, 8, 4, 0, PLL3_PFD0},
{16, 18, 6, 5, 0, PLL3_PFD0},
{17, 18, 7, 4, 0, PLL3_PFD0},
{18, 15, 8, 4, 0, PLL3_PFD0},
{19, 15, 6, 5, 0, PLL3_PFD0},
{20, 18, 6, 4, 0, PLL3_PFD0},
{21, 17, 6, 4, 0, PLL3_PFD0},
{22, 14, 7, 4, 0, PLL3_PFD0},
{23, 25, 5, 3, 0, PLL3_PFD0},
{24, 15, 6, 4, 0, PLL3_PFD0},
{25, 23, 5, 3, 0, PLL3_PFD0},
{26, 22, 5, 3, 0, PLL3_PFD0},
{27, 16, 5, 4, 0, PLL3_PFD0},
{28, 22, 7, 2, 0, PLL3_PFD0},
{29, 14, 7, 3, 0, PLL3_PFD0},
{30, 16, 6, 3, 0, PLL3_PFD0},
{31, 23, 4, 3, 0, PLL3_PFD0},
{32, 15, 6, 3, 0, PLL3_PFD0},
{33, 13, 5, 4, 0, PLL3_PFD0},
{34, 14, 6, 3, 0, PLL3_PFD0},
{35, 35, 7, 1, 0, PLL3_PFD0},
{36, 15, 8, 2, 0, PLL3_PFD0},
{37, 29, 4, 2, 0, PLL3_PFD0},
{38, 15, 5, 3, 0, PLL3_PFD0},
{39, 22, 5, 2, 0, PLL3_PFD0},
{40, 18, 4, 3, 0, PLL3_PFD0},
{41, 14, 5, 3, 0, PLL3_PFD0},
{42, 17, 4, 3, 0, PLL3_PFD0},
{43, 20, 5, 2, 0, PLL3_PFD0},
{44, 14, 7, 2, 0, PLL3_PFD0},
{45, 16, 4, 3, 0, PLL3_PFD0},
{46, 31, 3, 2, 0, PLL3_PFD0},
{47, 13, 7, 2, 0, PLL3_PFD0},
{48, 15, 4, 3, 0, PLL3_PFD0},
{49, 22, 4, 2, 0, PLL3_PFD0},
{50, 17, 5, 2, 0, PLL3_PFD0},
{51, 14, 4, 3, 0, PLL3_PFD0},
{52, 33, 5, 1, 0, PLL3_PFD0},
{53, 27, 3, 2, 0, PLL3_PFD0},
{54, 16, 5, 2, 0, PLL3_PFD0},
{55, 13, 4, 3, 0, PLL3_PFD0},
{56, 22, 7, 1, 0, PLL3_PFD0},
{57, 15, 5, 2, 0, PLL3_PFD0},
{58, 21, 7, 1, 0, PLL3_PFD0},
{59, 29, 5, 1, 0, PLL3_PFD0},
{60, 18, 4, 2, 0, PLL3_PFD0},
{61, 14, 5, 2, 0, PLL3_PFD0},
{62, 23, 3, 2, 0, PLL3_PFD0},
{63, 17, 4, 2, 0, PLL3_PFD0},
{64, 27, 5, 1, 0, PLL3_PFD0},
{65, 22, 3, 2, 0, PLL3_PFD0},
{66, 13, 5, 2, 0, PLL3_PFD0},
{67, 16, 4, 2, 0, PLL3_PFD0},
{68, 18, 7, 1, 0, PLL3_PFD0},
{69, 25, 5, 1, 0, PLL3_PFD0},
{70, 27, 5, 1, 0, PLL2_PFD2_CLK},
{71, 19, 7, 1, 0, PLL2_PFD2_CLK},
{72, 15, 4, 2, 0, PLL3_PFD0},
{73, 13, 5, 2, 0, PLL2_PFD2_CLK},
{74, 29, 4, 1, 0, PLL3_PFD0},
{75, 23, 5, 1, 0, PLL3_PFD0},
{76, 25, 5, 1, 0, PLL2_PFD2_CLK},
{77, 14, 4, 2, 0, PLL3_PFD0},
{78, 22, 5, 1, 0, PLL3_PFD0},
{79, 15, 4, 2, 0, PLL2_PFD2_CLK},
{80, 18, 3, 2, 0, PLL3_PFD0},
{81, 29, 4, 1, 0, PLL2_PFD2_CLK},
{82, 15, 7, 1, 0, PLL3_PFD0},
{83, 13, 4, 2, 0, PLL3_PFD0},
{84, 17, 3, 2, 0, PLL3_PFD0},
{85, 0, 1, 1, 7, SEMC_ROOT_CLK},
{86, 20, 5, 1, 0, PLL3_PFD0},
{87, 33, 3, 1, 0, PLL3_PFD0},
{88, 14, 7, 1, 0, PLL3_PFD0},
{90, 16, 3, 2, 0, PLL3_PFD0},
{91, 13, 4, 2, 0, PLL2_PFD2_CLK},
{92, 31, 3, 1, 0, PLL3_PFD0},
{93, 23, 4, 1, 0, PLL3_PFD0},
{94, 13, 7, 1, 0, PLL3_PFD0},
{95, 20, 5, 1, 0, PLL2_PFD2_CLK},
{96, 15, 3, 2, 0, PLL3_PFD0},
{98, 22, 4, 1, 0, PLL3_PFD0},
{99, 29, 3, 1, 0, PLL3_PFD0},
{100, 19, 5, 1, 0, PLL2_PFD2_CLK},
{101, 17, 5, 1, 0, PLL3_PFD0},
{102, 14, 3, 2, 0, PLL3_PFD0},
{103, 23, 4, 1, 0, PLL2_PFD2_CLK},
{104, 13, 7, 1, 0, PLL2_PFD2_CLK},
{105, 15, 3, 2, 0, PLL2_PFD2_CLK},
{106, 27, 3, 1, 0, PLL3_PFD0},
{108, 16, 5, 1, 0, PLL3_PFD0},
{109, 29, 3, 1, 0, PLL2_PFD2_CLK},
{110, 13, 3, 2, 0, PLL3_PFD0},
{111, 17, 5, 1, 0, PLL2_PFD2_CLK},
{113, 19, 4, 1, 0, PLL3_PFD0},
{115, 15, 5, 1, 0, PLL3_PFD0},
{117, 27, 3, 1, 0, PLL2_PFD2_CLK},
{118, 16, 5, 1, 0, PLL2_PFD2_CLK},
{120, 18, 4, 1, 0, PLL3_PFD0},
{121, 13, 3, 2, 0, PLL2_PFD2_CLK},
{123, 14, 5, 1, 0, PLL3_PFD0},
{125, 23, 3, 1, 0, PLL3_PFD0},
{126, 15, 5, 1, 0, PLL2_PFD2_CLK},
{127, 17, 4, 1, 0, PLL3_PFD0},
{130, 22, 3, 1, 0, PLL3_PFD0},
{132, 13, 5, 1, 0, PLL3_PFD0},
{135, 16, 4, 1, 0, PLL3_PFD0},
{137, 21, 3, 1, 0, PLL3_PFD0},
{139, 31, 2, 1, 0, PLL3_PFD0},
{144, 15, 4, 1, 0, PLL3_PFD0},
{146, 13, 5, 1, 0, PLL2_PFD2_CLK},
{148, 29, 2, 1, 0, PLL3_PFD0},
{150, 21, 3, 1, 0, PLL2_PFD2_CLK},
{151, 19, 3, 1, 0, PLL3_PFD0},
{153, 31, 2, 1, 0, PLL2_PFD2_CLK},
{154, 14, 4, 1, 0, PLL3_PFD0},
{158, 15, 4, 1, 0, PLL2_PFD2_CLK},
{160, 18, 3, 1, 0, PLL3_PFD0},
{163, 29, 2, 1, 0, PLL2_PFD2_CLK},
{166, 19, 3, 1, 0, PLL2_PFD2_CLK}, /*{166, 13, 4, 1, 0, PLL3_PFD0},*/
{170, 14, 4, 1, 0, PLL2_PFD2_CLK}, /*{169, 14, 4, 1, 0, PLL2_PFD2_CLK}  {169, 17, 3, 1, 0, PLL3_PFD0},*/
/*{172, 25, 2, 1, 0, PLL3_PFD0},*/
{176, 18, 3, 1, 0, PLL2_PFD2_CLK},
{180, 16, 3, 1, 0, PLL3_PFD0},
{182, 13, 4, 1, 0, PLL2_PFD2_CLK},
{186, 17, 3, 1, 0, PLL2_PFD2_CLK},
{187, 23, 2, 1, 0, PLL3_PFD0},
{190, 25, 2, 1, 0, PLL2_PFD2_CLK},
{192, 15, 3, 1, 0, PLL3_PFD0},
{196, 22, 2, 1, 0, PLL3_PFD0},
{198, 16, 3, 1, 0, PLL2_PFD2_CLK},
{205, 14, 3, 1, 0, PLL3_PFD0},
{206, 23, 2, 1, 0, PLL2_PFD2_CLK},
{211, 15, 3, 1, 0, PLL2_PFD2_CLK},
{216, 20, 2, 1, 0, PLL3_PFD0},
{221, 13, 3, 1, 0, PLL3_PFD0},
{226, 14, 3, 1, 0, PLL2_PFD2_CLK},
{227, 19, 2, 1, 0, PLL3_PFD0},
{237, 20, 2, 1, 0, PLL2_PFD2_CLK},
{240, 18, 2, 1, 0, PLL3_PFD0},
{243, 13, 3, 1, 0, PLL2_PFD2_CLK},
{250, 19, 2, 1, 0, PLL2_PFD2_CLK},
{254, 17, 2, 1, 0, PLL3_PFD0},
{264, 18, 2, 1, 0, PLL2_PFD2_CLK},
{270, 16, 2, 1, 0, PLL3_PFD0},
{279, 17, 2, 1, 0, PLL2_PFD2_CLK},
{288, 15, 2, 1, 0, PLL3_PFD0},
{297, 16, 2, 1, 0, PLL2_PFD2_CLK},
{308, 14, 2, 1, 0, PLL3_PFD0},
{316, 15, 2, 1, 0, PLL2_PFD2_CLK},
{332, 13, 2, 1, 0, PLL3_PFD0}
};

__attribute__ ((section(".ramfunc.$RAM")))
__inline__ void GetFlexSpiFreqFracPodfDiv_L(uint32_t* freq, uint8_t* frac, uint8_t* podf, uint8_t* div, bool dtrMode, uint8_t* semc_podf, uint8_t* pllType)
{
    bool foundFreq = false;
    uint32_t tmpFreq = *freq;
    if (dtrMode == true)
    {
        if (tmpFreq > NXP_105x_MAX_DTR_FREQ)
        {
            tmpFreq = NXP_105x_MAX_DTR_FREQ * 2;
        }
        else
        {
            tmpFreq = (uint32_t)(tmpFreq * 2);
        }
    }
    else
    {
        if (tmpFreq > NXP_105x_MAX_SDR_FREQ)
        {
            tmpFreq = NXP_105x_MAX_SDR_FREQ;
        }
    }
    for (int i=0 ; i < FLEX_SPI_FREQS_COUNT_ARRAY; i++)
    {
        if (FlexSpiFreqPodfFracDivArray[i].freq >= tmpFreq)
        {
            *freq = FlexSpiFreqPodfFracDivArray[i].freq;
            *frac = FlexSpiFreqPodfFracDivArray[i].frac;
            *podf = FlexSpiFreqPodfFracDivArray[i].podf;
            *div = FlexSpiFreqPodfFracDivArray[i].div;
            *semc_podf = FlexSpiFreqPodfFracDivArray[i].semc_podf;
            *pllType = FlexSpiFreqPodfFracDivArray[i].pllType;
            foundFreq = true;
            break;
        }
    }
    if (foundFreq == false)
    {
        *freq = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY-1)].freq;
        *frac = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY-1)].frac;
        *podf = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY-1)].podf;
        *div = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].div;
        *semc_podf = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].semc_podf;
        *pllType = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].pllType;
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:     flexspi_clock_change                                                                      */
/*                                                                                                         */
/* Parameters:   base        - FLEXSPI_Type pointer for FLEXSPI instance                                   */
/*               freq        - SPI frequency value from SPI_FREQUENCY_T ENUM                               */
/*               dtrMode     - boolean parameter for DTR configuration                                     */
/*               definedFreq - return frequency reached by hardware                                        */
/*               toPrint     - boolean parameter to enable/disable log print                               */
/* Returns:                                                                                                */
/* Side effects: none                                                                                      */
/* Description:  This function is used to change SPI frequency.                                            */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_clock_change(FLEXSPI_Type *base, uint32_t freq, bool dtrMode, uint32_t* definedFreq, bool toPrint)
{
    uint8_t frac, podf, div, pllType, semc_podf;
    uint32_t currentFreq = freq;
    uint32_t mcr0Val;

    GetFlexSpiFreqFracPodfDiv_L(&currentFreq, &frac, &podf, &div, dtrMode, &semc_podf, &pllType);
    currentFreq = (dtrMode == true) ? (currentFreq/2) : currentFreq;
    if (definedFreq!= NULL)
    {
        *definedFreq = currentFreq;
    }
    /* Wait for bus to be idle before changing flash configuration. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

    FLEXSPI_Enable(base, false);

    CLOCK_DisableClock(kCLOCK_SimEms);
    CLOCK_DisableClock(kCLOCK_FlexSpi);

    if (pllType == PLL3_PFD0) // Get root clock 480MHz and divide using frac values of 12-35
    {
        // Set PLL3 PFD0 clock fraction
        CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, frac);

        // Choose PLL3 PFD0 clock as flexspi source clock
        CLOCK_SetMux(kCLOCK_FlexspiMux, PLL3_PFD0);
    }
    else if (pllType == SEMC_ROOT_CLK) // Get root clock 600MHz and divide using semc_podf values of 1-8
    {
        /* Disable Semc clock gate. */
        CLOCK_DisableClock(kCLOCK_Semc);
        /* Set SEMC_PODF. */
        CLOCK_SetDiv(kCLOCK_SemcDiv, semc_podf);
        /* Set Semc alt clock source. */
        CLOCK_SetMux(kCLOCK_SemcAltMux, 1);
        /* Set Semc clock source. */
        CLOCK_SetMux(kCLOCK_SemcMux, 0);
        /* Set Semc clock source for FlexSpi. */
        CLOCK_SetMux(kCLOCK_FlexspiMux, SEMC_ROOT_CLK);
        /* Enable Semc clock gate. */
        CLOCK_EnableClock(kCLOCK_Semc);
    }
    else if (pllType == PLL2_PFD2_CLK) // Get root clock 528MHz and divide using frac values of 12-35
    {
        CLOCK_InitSysPfd(kCLOCK_Pfd2, frac);
        CLOCK_SetMux(kCLOCK_FlexspiMux, PLL2_PFD2_CLK);
    }

    /* Set FLEXSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, podf - 1);

    CLOCK_EnableClock(kCLOCK_SimEms);
    CLOCK_EnableClock(kCLOCK_FlexSpi);

    /* Set FLEXSPI_SERCLKDIV only possible after Clock is enabled */
    mcr0Val = base->MCR0;
    mcr0Val &= ~FLEXSPI_MCR0_SERCLKDIV_MASK;
    mcr0Val |= FLEXSPI_MCR0_SERCLKDIV(div-1);
    base->MCR0  = mcr0Val;

    FLEXSPI_Enable(base, true);

    for (uint32_t delay = 100U; delay > 0U; delay--)
    {
        __NOP();
    }

    // Reset SPI
    FLEXSPI_SoftwareReset(base);

    /* Wait at least 100 NOPs*/
    for (uint32_t delay = 1000U; delay > 0U; delay--)
    {
        __NOP();
    }

    /* Wait for bus to be idle before continue. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

    if ((g_spi_freq != currentFreq) || (g_dtr_mode != dtrMode))
    {
        flexspi_enable_dqs(base, currentFreq, g_dqs_mode, dtrMode);
    }
#ifdef WINBOND_DEBUG
        if(toPrint)
            PRINTF("FlexSPI Clock = %u MHz", currentFreq);
#endif //WINBOND_DEBUG
    g_spi_freq = currentFreq;
    g_dtr_mode = dtrMode;
    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:     flexspi_clock_init                                                                        */
/*                                                                                                         */
/* Parameters:   base        - FLEXSPI_Type pointer for FLEXSPI instance                                   */
/*               freq        - SPI frequency value from SPI_FREQUENCY_T ENUM                               */
/*               enableDQS   - enable DQS                                                                  */
/*                             Data Strobe (DS/DQS) pin is an output pin signal used to synchronize        */
/*                             data output for the host at higher SPI clock rates. Used only for reads.    */
/*               dtrMode     - boolean parameter for DTR configuration                                     */
/*               definedFreq - return frequency reached by hardware                                        */
/*               toPrint     - boolean parameter to enable/disable log print                               */
/* Returns:                                                                                                */
/* Side effects: none                                                                                      */
/* Description:  This function is used to initialize SPI frequency.                                        */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_clock_init(FLEXSPI_Type *base, uint32_t freq, bool enableDQS, bool dtrMode, uint32_t* definedFreq, bool toPrint)
{
    const clock_usb_pll_config_t g_ccmConfigUsbPll = {.loopDivider = 0U};

    CLOCK_InitUsb1Pll(&g_ccmConfigUsbPll);

    CLOCK_InitUsb2Pll(&g_ccmConfigUsbPll);


    flexspi_clock_change(base, freq, dtrMode, definedFreq, toPrint);
#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Can't replace MCU fetch command when direct access is enabled
    if (!IS_LUT_INITIALIZED)
    {
        flexspi_secure_LUT_init(QLIB_BUS_MODE_1_1_1, dtrMode);
    }
    else
    {
        if (dtrMode != g_LUT_DTR)
        {
            flexspi_secure_LUT_init(g_LUT_format, dtrMode);
        }

    }
#endif // QLIB_NO_DIRECT_FLASH_ACCESS
#endif // SPI_LUT_OPTIMIZATION_ENABLED
    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        flexspi_enable_dqs                                                                     */
/*                                                                                                         */
/* Parameters:      base      - FLEXSPI_Type pointer for FLEXSPI instance                                  */
/*                  freq      - SPI frequency value from SPI_FREQUENCY_T ENUM                              */
/*                  enableDQS - enable DQS                                                                 */
/*                              Data Strobe (DS/DQS) pin is an output pin signal used to synchronize       */
/*                              data output for the host at higher SPI clock rates. Used only for reads.   */
/*                  dtrMode   - boolean parameter for DTR configuration                                    */
/* Returns:                                                                                                */
/* Side effects:    none                                                                                   */
/* Description:     This function is used to change DQS NXP functionality                                  */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_enable_dqs(FLEXSPI_Type *base, uint32_t freq, bool enableDqs, bool dtrMode)
{
    const uint8_t port = 0; /* PortA with index 0, PortB with index 1. */
    uint32_t flexspiDllValue = 0x0;
    uint32_t statusValue = 0;
    uint32_t dllValue = 0;
    bool useflexspiDllLockForDqs = false;
    float waveInNanoSeconds = 0.0;
    /* Wait for bus to be idle before changing flash configuration. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

    /* Step into stop mode. */
    base->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;


    // Calculate the waveInNanoSeconds, meaning the time in [ns] from last dummy cycle clock edge,
    // from which the data output should be generated, to the first trigger point to sample the data,
    // In DTR the sample point is at the first raising edge, and in SDR, first falling edge
    // and also represent the wave length T[ns] for the Root clock used in DQS DLL lock.
    waveInNanoSeconds = (dtrMode) ? (500.0/(float)freq) : (1000.0/(float)freq);
    if (enableDqs)
    {
        // DQS enabled 
        if (((dtrMode == true) && ((freq * 2) >= SPI_FREQUENCY_100MHz)) ||
                (freq > SPI_FREQUENCY_100MHz))
        {
            // Frequency above 100Mhz, use DLL lock and set dllValue to 1/4T=7, as T=32
            // In DTR sampling timing is set at 1/4T = 7,
            // In SDR sampling timing is set at falling edge of DQS, value = 0.
            dllValue = (dtrMode == true) ? 7 : 0;
            useflexspiDllLockForDqs = true;

            // g_tdssq[ns], setup time from DQS edge to valid data
            // When the DQS is enabled, the sampling point is set as dllValue 0-31, per quanta percentage of T
            // for example, dllValue=16 1/2T-rootClk.
            // used to let the user set modified sampling point as cross-talk interference can effect performance,
            // in high operating frequencies.
            if (g_tdssq > 0.0)
            {
                if ((g_tdssq < waveInNanoSeconds) && (dtrMode == true))
                {
                    dllValue = (uint32_t)((g_tdssq / waveInNanoSeconds) * 32.0) + 2;
                }
            }
        }
        else
        {
           // Frequency below 100Mhz, DLL lock is not used, calculate offset from DQS edge
           // as quanta of dllValue 0-63 values, per quanta, 225ps delay.
            dllValue = (uint32_t)(((waveInNanoSeconds * 1000.0) / 4.0 ) / 225.0) + 1;
        }
    }
    else
    {
        // When the DQS is not enable, the DLL lock is not used, and sampling point is calculated,
        // as quanta of dllValue 0-63 values, per quanta, 225ps delay.
        if (dtrMode == true)
        {
            // g_tclqv[ns], setup time from clock edge to valid data
            // if g_tclqv[ns] > waveInNanoSeconds, need to add more delay to sample point timing,
            // to sample the data when it is valid.
            // N[dllValue] = ((g_tdsu[ns]-waveInNanoSeconds[ns])*1000)[ps]/225ps
            if (g_tclqv > waveInNanoSeconds)
            {
                dllValue = (uint32_t)(((g_tclqv-waveInNanoSeconds)*1000.0) / 225.0)+1;
            }
        }
    }

    if (useflexspiDllLockForDqs == false)
    {
        if (dllValue > 0)
        {
            dllValue = ((dllValue-1) > 63) ? 63 : (dllValue-1);
        }
        flexspiDllValue = FLEXSPI_DLLCR_OVRDEN(1) | FLEXSPI_DLLCR_DLLEN(0) | FLEXSPI_DLLCR_OVRDVAL(dllValue);
    }
    else
    {
        if (dllValue > 0)
        {
            dllValue = ((dllValue-1) > 31) ? 31 : (dllValue-1);
        }
        flexspiDllValue = FLEXSPI_DLLCR_OVRDEN(0) | FLEXSPI_DLLCR_DLLEN(1) | FLEXSPI_DLLCR_SLVDLYTARGET(dllValue);
    }

    base->DLLCR[port] = flexspiDllValue;
    /* Set FlexSPI clk src */
    base->MCR0 &= ~(FLEXSPI_MCR0_RXCLKSRC_MASK);
    if (enableDqs)
    {
        /* Set FlexSPI clk src to DQS */
        base->MCR0 |= FLEXSPI_MCR0_RXCLKSRC(kFLEXSPI_ReadSampleClkExternalInputFromDqsPad);
        g_dqs_mode = true;
    }
    else
    {
        base->MCR0 |= FLEXSPI_MCR0_RXCLKSRC(FLEXSPI_RX_SAMPLE_CLK);
        g_dqs_mode = false;
    }

    /* Exit stop mode. */
    base->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    statusValue =
        ((uint32_t)kFLEXSPI_FlashASampleClockSlaveDelayLocked | (uint32_t)kFLEXSPI_FlashASampleClockRefDelayLocked);

    if (0U != (flexspiDllValue & FLEXSPI_DLLCR_DLLEN_MASK))
    {
        /* Wait slave delay line locked and slave reference delay line locked. */
        while ((base->STS2 & statusValue) != statusValue)
        {
        }

        /* Wait at least 100 NOPs*/
        for (uint8_t delay = 100U; delay > 0U; delay--)
        {
            __NOP();
        }
    }

    /* Wait at least 100 NOPs*/
    for (uint32_t delay = 1000U; delay > 0U; delay--)
    {
        __NOP();
    }

    /* Wait for bus to be idle before continue. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        flexspi_getfreq                                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/* Returns:                                                                                                */
/* Side effects:    none                                                                                   */
/* Description:     This function returns the configured SPI frequency.                                    */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
uint32_t flexspi_getfreq(void)
{
    return (uint32_t)g_spi_freq;
}

__attribute__ ((section(".ramfunc.$RAM")))
bool flexspi_getDqsMode(void)
{
    return g_dqs_mode;
}
/*---------------------------------------------------------------------------------------------------------*/
/* Function:        flexspi_nor_flash_init                                                                 */
/*                                                                                                         */
/* Parameters:      base     - FLEXSPI_Type pointer for FLEXSPI instance                                   */
/* Returns:                                                                                                */
/* Side effects:    none                                                                                   */
/* Description:     This function is used to initialize FLEXSPI flash interface.                           */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_nor_flash_init(FLEXSPI_Type *base)
{
    flexspi_config_t config;

    // Get FLEXSPI default settings and configure the flexspi
    FLEXSPI_GetDefaultConfig(&config);

    // Set AHB buffer size for reading data through AHB bus
    config.ahbConfig.enableAHBPrefetch = true;
    config.ahbConfig.enableAHBBufferable = true;
    config.ahbConfig.enableReadAddressOpt = true;
    config.ahbConfig.enableAHBCachable = true;
    config.rxSampleClock = FLEXSPI_RX_SAMPLE_CLK;
    config.enableCombination = true;

    FLEXSPI_Init(base, &config);

    // Configure flash settings according to serial flash feature
    FLEXSPI_SetFlashConfig(base, &deviceConfig, kFLEXSPI_PortA1);

    // Update LUT table
    FLEXSPI_UpdateLUT(base, 0, customLUT, CUSTOM_LUT_LENGTH);

    // Reset SPI
    base->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;
    FLEXSPI_SoftwareReset(base);
    base->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;

    // Initialize FlexSPI PLL
    flexspi_clock_init(FLEXSPI, SPI_FREQUENCY_4MHz, DQS_OFF, DTR_OFF, NULL, false);

    return 0;
}

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
/*---------------------------------------------------------------------------------------------------------*/
/* Function:        flexspi_secure_LUT_init                                                                */
/*                                                                                                         */
/* Parameters:      dtrMode     - select LUT according for DTR or SDR mode                                 */
/*                  format      - SPI bus format                                                           */
/* Returns:                                                                                                */
/* Side effects:    none                                                                                   */
/* Description:     This function is used to initialize FLEXSPI LUT with secure commands                   */
/*---------------------------------------------------------------------------------------------------------*/
status_t flexspi_secure_LUT_init(QLIB_BUS_MODE_T format, bool dtrMode)
{
    // Update LUT table with single SPI instructions
    FLEXSPI_UpdateLUT(FLEXSPI, 0, W77Q_SINGLE_SDR_DTR_LUT, CUSTOM_LUT_LENGTH);

    memset(g_SequenceTable, LUT_SEQUENCE_TO_USE, 256);
    g_SequenceTable[0x05] = W77Q_LUT_SEQ_IDX_1_1_1_READ_STATUS_REG1;
    g_SequenceTable[0xA0] = W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP0;
    g_SequenceTable[0xA1] = W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1;
    g_SequenceTable[0xA2] = W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP2;
    g_SequenceTable[0xA4] = W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP0;
    g_SequenceTable[0xA6] = W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP2;
    g_SequenceTable[SPI_FLASH_CMD__ENTER_SPI] = W77Q_LUT_SEQ_IDX_4_4_4_ENTER_SPI;
    g_SequenceTable[SPI_FLASH_CMD__ENTER_QPI] = W77Q_LUT_SEQ_IDX_1_1_1_ENTER_QPI;

    if (format != QLIB_BUS_MODE_1_1_1)
    {
        flexspi_secure_LUT_update_L(format, dtrMode);
    }

    g_LUT_format = format;
    if (format == QLIB_BUS_MODE_8_8_8)
    {
        g_LUT_DTR = dtrMode;
    }

    return 0;
}
#endif

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        flexspi_default_LUT_init                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/* Returns:                                                                                                */
/* Side effects:    none                                                                                   */
/* Description:     This function is used to initialize FLEXSPI LUT with secure commands                   */
/*---------------------------------------------------------------------------------------------------------*/
void flexspi_default_LUT_init()
{
    FLEXSPI_UpdateLUT(FLEXSPI, 0, customLUT, CUSTOM_LUT_LENGTH);

    g_LUT_DTR = false;
    g_LUT_format = QLIB_BUS_MODE_INVALID;

}

__attribute__ ((section(".ramfunc.$RAM")))
inline uint32_t flexspi_get_fetch_cmd_lut_idx(void)
{
    return NOR_CMD_LUT_SEQ_IDX_CODE_FETCH_CMD;
}

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
void flexspi_secure_LUT_update_L(QLIB_BUS_MODE_T format, bool dtrMode)
{
    flexspi_pad_t dataLines;
    flexspi_pad_t cmdLines;
    flexspi_pad_t addrLines;

    enum _flexspi_command flexspiCmd =
#ifdef QLIB_SUPPORT_OPI
        ((format == QLIB_BUS_MODE_8_8_8) && (dtrMode == true)) ? kFLEXSPI_Command_DDR :
#endif
            kFLEXSPI_Command_SDR;
    bool op1Dtr =     // OP1 in Q2 does not support DTR. in Q3 DTR is supported only in DOPI mode
#ifdef QLIB_SUPPORT_OPI
        ((format == QLIB_BUS_MODE_8_8_8) && (dtrMode == true)) ? true :
#endif
            false;

    switch (format)
        {
            case QLIB_BUS_MODE_1_1_1:
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_1PAD;
                dataLines = kFLEXSPI_1PAD;
                break;
#ifdef QLIB_SUPPORT_DUAL_SPI
            case QLIB_BUS_MODE_1_1_2:
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_1PAD;
                dataLines = kFLEXSPI_2PAD;
                break;
            case QLIB_BUS_MODE_1_2_2:
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_2PAD;
                dataLines = kFLEXSPI_2PAD;
                break;
#endif
            case QLIB_BUS_MODE_1_1_4:
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_1PAD;
                dataLines = kFLEXSPI_4PAD;
                break;
            case QLIB_BUS_MODE_1_4_4:
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_4PAD;
                dataLines = kFLEXSPI_4PAD;
                break;
#ifdef QLIB_SUPPORT_QPI
            case QLIB_BUS_MODE_4_4_4:
                cmdLines = kFLEXSPI_4PAD;
                addrLines = kFLEXSPI_4PAD;
                dataLines = kFLEXSPI_4PAD;
                break;
#endif
            case QLIB_BUS_MODE_1_8_8:
                            cmdLines = kFLEXSPI_1PAD;
                            addrLines = kFLEXSPI_8PAD;
                            dataLines = kFLEXSPI_8PAD;
                            break;
#ifdef QLIB_SUPPORT_OPI
            case QLIB_BUS_MODE_8_8_8:
                            cmdLines = kFLEXSPI_8PAD;
                            addrLines = kFLEXSPI_8PAD;
                            dataLines = kFLEXSPI_8PAD;
                            break;
#endif
            default:
                //single 1_1_1
                cmdLines = kFLEXSPI_1PAD;
                addrLines = kFLEXSPI_1PAD;
                dataLines = kFLEXSPI_1PAD;
                break;
        }

    #define OP0_SEC_CMD(format, dtr) W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP0, format, dtr == true ? 1 : 0)
    #define OP1_SEC_CMD(format, dtr) W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP1, format, dtr == true ? 1 : 0)
    #define OP2_SEC_CMD(format, dtr) W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP2, format, dtr == true ? 1 : 0)

    // OP0
    uint32_t op0SdrCmd[] =
    {
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, cmdLines, OP0_SEC_CMD(format, false),
                kFLEXSPI_Command_DUMMY_SDR, dataLines, SEC_CMD_OP0_SDR_DUMMY_CYCLES),
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR,  dataLines, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        0,
        0
    };

    uint32_t op0DtrCmd[] =
    {
        FLEXSPI_LUT_SEQ(format == QLIB_BUS_MODE_8_8_8 ? kFLEXSPI_Command_DDR : kFLEXSPI_Command_SDR, cmdLines, OP0_SEC_CMD(format, true),
                kFLEXSPI_Command_DUMMY_DDR, dataLines, (2 * SEC_CMD_OP0_DTR_DUMMY_CYCLES)),
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR,  dataLines, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        0,
        0
    };


    // OP1
    uint32_t op1Cmd[] =
    {
        FLEXSPI_LUT_SEQ(flexspiCmd, cmdLines, OP1_SEC_CMD(format, op1Dtr),
                op1Dtr == true ? kFLEXSPI_Command_RADDR_DDR : kFLEXSPI_Command_RADDR_SDR, addrLines, SEC_CMD_OP1_CTAG_SIZE),
        0,
        0,
        0
    };


    // OP1 with data
    uint32_t op1DataCmd[] =
    {
        FLEXSPI_LUT_SEQ(flexspiCmd, cmdLines, OP1_SEC_CMD(format, op1Dtr),
                op1Dtr == true ? kFLEXSPI_Command_RADDR_DDR : kFLEXSPI_Command_RADDR_SDR, addrLines, SEC_CMD_OP1_CTAG_SIZE),
        FLEXSPI_LUT_SEQ(op1Dtr == true ? kFLEXSPI_Command_WRITE_DDR : kFLEXSPI_Command_WRITE_SDR, dataLines, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        0,
        0
    };

    //OP2 SDR
    uint32_t op2SdrCmd[] =
    {
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, cmdLines, OP2_SEC_CMD(format, false),
                kFLEXSPI_Command_DUMMY_SDR, dataLines, SEC_CMD_OP2_DUMMY_CYCLES),
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR,  dataLines, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        0,
        0
    };

    //OP2 DTR
    uint32_t op2DtrCmd[] =
    {
        FLEXSPI_LUT_SEQ(format == QLIB_BUS_MODE_8_8_8 ? kFLEXSPI_Command_DDR : kFLEXSPI_Command_SDR, cmdLines, OP2_SEC_CMD(format, true),
                kFLEXSPI_Command_DUMMY_DDR, dataLines, (2 * SEC_CMD_OP2_DUMMY_CYCLES)),
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR,  dataLines, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        0,
        0
    };

    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP0, op0SdrCmd, 4);
    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP0, op0DtrCmd, 4);
    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_OP1, op1Cmd, 4);
    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_OP1_Data, op1DataCmd, 4);
    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP2, op2SdrCmd, 4);
    FLEXSPI_UpdateLUT(FLEXSPI, 4 * W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP2, op2DtrCmd, 4);

    // clear previous secure operations from LUT (single is always in LUT - don't clean it)
    g_SequenceTable[0xB4] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xD4] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xB6] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xD6] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xB0] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xD0] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xB1] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xD1] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xB2] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xD2] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xF0] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xF1] = LUT_SEQUENCE_TO_USE;
    g_SequenceTable[0xF2] = LUT_SEQUENCE_TO_USE;

    // update new secure operations
    if (format != QLIB_BUS_MODE_1_1_1)
    {
        g_SequenceTable[OP1_SEC_CMD(format, op1Dtr)] = W77Q_LUT_SEQ_IDX_DYNAMIC_OP1;
        if (format != QLIB_BUS_MODE_8_8_8)
        {
            g_SequenceTable[OP0_SEC_CMD(format, false)] = W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP0;
            g_SequenceTable[OP0_SEC_CMD(format, true)] = W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP0;
            g_SequenceTable[OP2_SEC_CMD(format, false)] = W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP2;
            g_SequenceTable[OP2_SEC_CMD(format, true)] = W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP2;
        }
        else
        {
            // secure command opcode for DOPI and SOPI is the same, so choose only one
            if (dtrMode == false)
            {
                g_SequenceTable[OP0_SEC_CMD(format, false)] = W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP0;
                g_SequenceTable[OP2_SEC_CMD(format, false)] = W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP2;
            }
            else
            {
                g_SequenceTable[OP0_SEC_CMD(format, true)] = W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP0;
                g_SequenceTable[OP2_SEC_CMD(format, true)] = W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP2;
            }
        }
    }
}
#endif

// Tclqv is Clock to Output Data Valid time in nanoseconds.
// This value is used to correctly modify the sampling timing.
// Need to call flexspi_enable_dqs function, to update sampling time.
// flexspi_enable_dqs function is called on change frequency.
void flexspi_set_tclqv(float tclqv)
{
    if (tclqv > 0.0)
    {
        g_tclqv = tclqv;
    }
}

// Tdsu is Data In Setup Time to CLK rising edge time in nanoseconds.
// This value is used to correctly modify the sampling timing.
// Need to call flexspi_enable_dqs function, to update sampling time.
// flexspi_enable_dqs function is called on change frequency.
void flexspi_set_tdssq(float tdssq)
{
    if (tdssq > 0.0)
    {
        g_tdssq = tdssq;
    }
}
