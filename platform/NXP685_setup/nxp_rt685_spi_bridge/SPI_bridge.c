/*---------------------------------------------------------------------------------------------------------*/
/* Winbond Electronics Corporation                                                                         */
/*                                                                                                         */
/*                                                                                                         */
/* Copyright (c) 2019 by Winbond Electronics Corporation                                                   */
/* All rights reserved                                                                                     */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
/* File Content:                                                                                           */
/*            This file contains NXP RT685 FlexSpi Flash function implementation                           */
/* Project:                                                                                                */
/*            NXP RT685                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#include "SPI_bridge.h"
#include "defs_utils.h"
#include "qlib_platform.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NXP_rt685_MAX_SDR_FREQ                         600
#define NXP_rt685_MAX_DTR_FREQ                         300
#define WAITSEQ_MAX_READ_1024_MULTIPLE                (65)
#define NXP_1MHz_START_FREQ                            (1)
#define FLEXSPI_DLLCR_DEFAULT                    (0x100UL)
#define FLEXSPI_LUT_KEY_VAL                 (0x5AF05AF0UL)

#define LUT_SEQUENCE_TO_USE                           (15)
#define LUT_DYNAMIC_SEQUENCE_INDEX                    (15)
#define LUT_DYNAMIC_SEQUENCE_SIZE                     (4)
#define SPI_CMD_DTR(flags) (((flags) & QLIB_DTR_MASK) != QLIB_DTR__NO_DTR)

#define FLEXSPI_LUT_SEQ_SINGL_INST(cmd0, pad0, op0) FLEXSPI_LUT_SEQ(cmd0, pad0, op0, 0, 0, 0);
#define FLEXSPI_LUT_SEQ_MAKE_LINE(inst0, inst1) ((inst0) | ((inst1) << FLEXSPI_LUT_OPERAND1_SHIFT))

#define FLASH_PORT                                    kFLEXSPI_PortA1
#define FLEXSPI_RX_SAMPLE_CLOCK                       kFLEXSPI_ReadSampleClkLoopbackFromSckPad // kFLEXSPI_ReadSampleClkLoopbackInternally

#define FLEXSPI_SPI_CMD_PHASE                         (0)
#define FLEXSPI_SPI_ADDR_PHASE                        (1)
#define FLEXSPI_SPI_DATA_PHASE                        (2)

#define QLIB_SPI_FLAGS__MODE_BIT_SHIFT (16u)
#define QLIB_SPI_FLAGS__MODE_ENABLE    (1u << QLIB_SPI_FLAGS__MODE_BIT_SHIFT)
#define QLIB_SPI_FLAGS__MODE_SIZE_MASK (0x7)

#define QLIB_SPI_FLAGS__GET_MODE_BITS_SIZE(flags) (!(flags & QLIB_SPI_FLAGS__MODE_ENABLE)) ? 0 : \
                            (((flags >> (QLIB_SPI_FLAGS__MODE_BIT_SHIFT + 1)) & QLIB_SPI_FLAGS__MODE_SIZE_MASK) + 1)

enum
{
    kFLEXSPI_DelayCellUnitMin = 75,  /* 75ps. */
    kFLEXSPI_DelayCellUnitMax = 225, /* 225ps. */
};

enum
{
    kFLEXSPI_FlashASampleClockSlaveDelayLocked =
        FLEXSPI_STS2_ASLVLOCK_MASK, /* Flash A sample clock slave delay line locked. */
    kFLEXSPI_FlashASampleClockRefDelayLocked =
        FLEXSPI_STS2_AREFLOCK_MASK, /* Flash A sample clock reference delay line locked. */
    kFLEXSPI_FlashBSampleClockSlaveDelayLocked =
        FLEXSPI_STS2_BSLVLOCK_MASK, /* Flash B sample clock slave delay line locked. */
    kFLEXSPI_FlashBSampleClockRefDelayLocked =
        FLEXSPI_STS2_BREFLOCK_MASK, /* Flash B sample clock reference delay line locked. */
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
__attribute__ ((section(".ramfunc.$RAM")))
void GetFlexSpiFreqDivAndPllSource_L(uint32_t* freq, bool dtrMode, uint32_t* flexSpiDiv, uint32_t* aux0pfd, uint32_t* aux0Div, uint32_t* pllSource);
__attribute__ ((section(".ramfunc.$RAM")))
int NXP_writeReadTransaction(PLAT_SPI_FORMAT_T format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize);
__attribute__ ((section(".ramfunc.$RAM")))
int NXP_FLEXSPI_TransferBlocking_L(FLEXSPI_Type *base, flexspi_transfer_t *xfer);
__attribute__ ((section(".ramfunc.$RAM")))
int NXP_FLEXSPI_ReadBlocking_L(FLEXSPI_Type *base, uint8_t *buffer, size_t size);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_LUT_Update_Cnt = 0;
volatile uint8_t g_min_pad_count = PLAT_SPI_FORMAT_1_1_1;
bool g_dqs_mode = false;
bool g_set_dqs_mode = false;
bool g_dtr_mode = false;
bool g_dqs_pol = false;
bool g_reach_flexspi_timeout_interrupt_failure = false;
extern bool g_use_timeOut;
#ifdef STRESS_FROM_HOST
bool g_reduce_freq_bypass_for_infinite_read_busy_flag = false;
uint32_t g_reduce_freq_for_read_busy = 100;
#endif
float g_tclqv = 4.5;
float g_tdssq = 0.4;
uint32_t g_spi_freq = NXP_1MHz_START_FREQ;
uint32_t g_set_spi_freq = NXP_1MHz_START_FREQ;
volatile bool flexspi_irq_flag = false;
#if defined(EXAMPLE_FLASH_RESET_CONFIG)
extern const uint32_t FastReadSDRLUTCommandSeq[4];
extern const uint32_t OctalReadDDRLUTCommandSeq[4];
#endif

static flexspi_device_config_t deviceconfig = {
    .flexspiRootClk       = 49500000, /* max frequency 495MHz start frequency */
    .flashSize            = (4 * 1024 * 1024), /* define max address size for device size of 2GB for 32b address input and CTAG without restrictions */
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2, /* 2 minimum value */
    .CSHoldTime           = 1, /* Q3 TCSH 2ns */
    .CSSetupTime          = 1, /* Q3 TCSS 2.25ns */
    .dataValidTime        = 0,
    .columnspace          = 0,
    .enableWordAddress    = 0,
    .AWRSeqIndex          = 0, //NOR_CMD_LUT_SEQ_IDX_WRITE,
    .AWRSeqNumber         = 1,
    .ARDSeqIndex          = 0, //NOR_CMD_LUT_SEQ_IDX_READ,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};



/*******************************************************************************
 * Code
 ******************************************************************************/
__attribute__ ((section(".ramfunc.$RAM")))
void flexspi_nor_flash_init(FLEXSPI_Type *base)
{
    flexspi_config_t config;

    /* Attach AUX0_PLL clock to flexspi with divider 4*/
    //BOARD_SetFlexspiClock(2, 8);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /*Get FLEXSPI default settings and configure the flexspi. */
    FLEXSPI_GetDefaultConfig(&config);

    /*Set AHB buffer size for reading data through AHB bus. */
    config.ahbConfig.enableAHBPrefetch = true;
    config.rxSampleClock               = FLEXSPI_RX_SAMPLE_CLOCK;
#if !(defined(FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN) && FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN)
    config.enableCombination = true;
#endif
    config.ahbConfig.enableAHBBufferable = true;
    config.ahbConfig.enableAHBCachable   = true;
    //config.ahbConfig.ahbBusTimeoutCycle
    //config.seqTimeoutCycle = 0xC0;
    FLEXSPI_Init(base, &config);

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(base, &deviceconfig, FLASH_PORT);

    /* Copy LUT information from flash region into RAM region, because flash will be reset and back to single mode;
       In lately time, LUT table assignment maybe failed after flash reset due to LUT read entry is application's
       required mode(such as octal DDR mode) and flash is being in single SDR mode, they don't matched. */
#if defined(EXAMPLE_FLASH_RESET_CONFIG)
    uint32_t TempFastReadSDRLUTCommandSeq[4];

    memcpy(TempFastReadSDRLUTCommandSeq, FastReadSDRLUTCommandSeq, sizeof(FastReadSDRLUTCommandSeq));
#endif

    /* Update LUT table into a specific mode, such as octal SDR mode or octal DDR mode based on application's
     * requirement. */
    //FLEXSPI_UpdateLUT(base, 0, customLUTOctalMode, CUSTOM_LUT_LENGTH);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if (defined(XIP_EXTERNAL_FLASH) && XIP_EXTERNAL_FLASH == 1) && (FLASH_ADESTO == 1)
    status_t status;
    flexspi_transfer_t flashXfer;
    uint32_t tempLUT[4];

    /* Exit octal mode command. */
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0xFF, kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0x0);
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, 4);

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0, true);

    /* Back to standard SPI mode. */
    flashXfer.deviceAddress = 0;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_CONFIG;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status == kStatus_Success)
    {
        status = flexspi_nor_wait_bus_busy(base, false);
    }
#endif

#if defined(EXAMPLE_FLASH_RESET_CONFIG)
    EXAMPLE_FLASH_RESET_CONFIG();
    /* FAST_READ4B: back to single mode and update read LUT entry into FAST_READ4B after flash reset,
           to make sure fetch code/data from flash is right. */
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempFastReadSDRLUTCommandSeq, 4);
#endif

#if defined(EXAMPLE_INVALIDATE_FLEXSPI_CACHE)
    EXAMPLE_INVALIDATE_FLEXSPI_CACHE();
#endif

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    flexspi_clock_init(base, g_set_spi_freq, false, false, NULL);
}


#define FLEXSPI_CLK_SEL_FFRO_48_60_IRC 3
#define FLEXSPI_CLK_SEL_MAIN_PLL 1
#define FLEXSPI_CLK_SEL_AUX0_PLL 2


#define FLEX_SPI_FREQS_COUNT_ARRAY 148 // 148 individual parameters for frequencies between 1-400MHz
typedef struct
{
    uint32_t freq: 10;
    uint32_t flexSpiDiv: 8;
    uint32_t aux0pfd: 6;
    uint32_t aux0div: 5;
    uint32_t pllSource: 3;
} flexspi_param_freq_list_t;

// values generated using python script according to NXP rt685 Clock tree
// python script: nxp_rt685_calc_flexSpi_clk_config_values.py
static flexspi_param_freq_list_t FlexSpiFreqPodfFracDivArray[FLEX_SPI_FREQS_COUNT_ARRAY]= {
        {1, 48, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {1, 48, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {2, 24, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {3, 16, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {4, 12, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {5, 100, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {6, 8, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {7, 3, 18, 25, FLEXSPI_CLK_SEL_AUX0_PLL },
        {8, 6, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {9, 88, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {10, 50, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {11, 72, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {12, 4, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {13, 43, 17, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {14, 26, 13, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {15, 5, 14, 9, FLEXSPI_CLK_SEL_AUX0_PLL },
        {16, 3, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {17, 43, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {18, 44, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {19, 25, 20, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {20, 25, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {21, 10, 15, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {22, 36, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {23, 1, 27, 15, FLEXSPI_CLK_SEL_AUX0_PLL },
        {24, 2, 12, 1, FLEXSPI_CLK_SEL_FFRO_48_60_IRC },
        {25, 20, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {26, 28, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {27, 22, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {28, 26, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {29, 5, 13, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {30, 2, 13, 12, FLEXSPI_CLK_SEL_AUX0_PLL },
        {31, 1, 16, 19, FLEXSPI_CLK_SEL_AUX0_PLL },
        {32, 1, 21, 14, FLEXSPI_CLK_SEL_AUX0_PLL },
        {33, 24, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {34, 3, 23, 4, FLEXSPI_CLK_SEL_AUX0_PLL },
        {35, 18, 15, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {36, 22, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {37, 17, 15, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {38, 19, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {39, 1, 22, 11, FLEXSPI_CLK_SEL_AUX0_PLL },
        {40, 8, 29, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {41, 11, 21, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {42, 16, 14, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {43, 17, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {44, 18, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {45, 3, 14, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {46, 2, 34, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {47, 2, 25, 4, FLEXSPI_CLK_SEL_AUX0_PLL },
        {48, 11, 18, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {49, 16, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {50, 10, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {51, 6, 31, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {52, 3, 20, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {54, 11, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {55, 1, 17, 10, FLEXSPI_CLK_SEL_AUX0_PLL },
        {56, 13, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {57, 5, 33, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {58, 2, 27, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {59, 10, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {60, 3, 26, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {61, 5, 31, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {62, 9, 17, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {63, 10, 15, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {64, 1, 21, 7, FLEXSPI_CLK_SEL_AUX0_PLL },
        {65, 1, 29, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {66, 12, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {67, 1, 20, 7, FLEXSPI_CLK_SEL_AUX0_PLL },
        {68, 1, 23, 6, FLEXSPI_CLK_SEL_AUX0_PLL },
        {69, 2, 34, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {70, 9, 15, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {71, 1, 19, 7, FLEXSPI_CLK_SEL_AUX0_PLL },
        {72, 11, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {73, 10, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {74, 8, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {75, 9, 14, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {76, 5, 25, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {79, 5, 12, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {81, 1, 13, 9, FLEXSPI_CLK_SEL_AUX0_PLL },
        {82, 1, 23, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {83, 6, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {84, 4, 28, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {86, 5, 22, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {88, 9, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {91, 8, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {91, 4, 13, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {93, 6, 17, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {95, 5, 20, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {96, 3, 33, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {99, 8, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {100, 1, 19, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {102, 3, 31, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {103, 4, 23, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {104, 7, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {105, 1, 18, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {108, 4, 22, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {109, 3, 29, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {111, 1, 17, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {113, 7, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {117, 3, 27, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {118, 2, 20, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {121, 3, 26, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {125, 4, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {126, 3, 25, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {132, 6, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {135, 1, 14, 5, FLEXSPI_CLK_SEL_AUX0_PLL },
        {137, 1, 23, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {139, 2, 34, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {144, 3, 22, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {146, 5, 13, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {148, 4, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {150, 1, 21, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {153, 2, 31, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {158, 5, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {163, 1, 29, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {166, 3, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {169, 2, 28, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {176, 3, 18, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {182, 2, 26, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {186, 3, 17, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {190, 2, 25, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {198, 4, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {206, 1, 23, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {211, 3, 15, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {216, 2, 22, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {226, 2, 21, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {237, 1, 20, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {243, 1, 13, 3, FLEXSPI_CLK_SEL_AUX0_PLL },
        {250, 1, 19, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {264, 3, 12, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {279, 1, 17, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {288, 1, 33, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {297, 1, 32, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {306, 1, 31, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {316, 1, 15, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {327, 1, 29, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {339, 2, 14, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {352, 1, 27, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {365, 1, 13, 2, FLEXSPI_CLK_SEL_AUX0_PLL },
        {380, 1, 25, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {396, 1, 24, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {413, 1, 23, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {432, 1, 22, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {452, 1, 21, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {475, 1, 20, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {500, 1, 12, 1, FLEXSPI_CLK_SEL_MAIN_PLL },
        {528, 1, 18, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {559, 1, 17, 1, FLEXSPI_CLK_SEL_AUX0_PLL },
        {594, 1, 16, 1, FLEXSPI_CLK_SEL_AUX0_PLL }
};

__attribute__ ((section(".ramfunc.$RAM")))
void GetFlexSpiFreqDivAndPllSource_L(uint32_t* freq, bool dtrMode, uint32_t* flexSpiDiv, uint32_t* aux0pfd, uint32_t* aux0Div, uint32_t* pllSource)
{
    bool foundFreq = false;
    uint32_t tmpFreq = *freq;
    if (dtrMode == true)
    {
        if (tmpFreq > NXP_rt685_MAX_DTR_FREQ)
        {
            tmpFreq = NXP_rt685_MAX_DTR_FREQ * 2;
        }
        else
        {
            tmpFreq = (uint32_t)(tmpFreq * 2);
        }
    }
    else
    {
        if (tmpFreq > NXP_rt685_MAX_SDR_FREQ)
        {
            tmpFreq = NXP_rt685_MAX_SDR_FREQ;
        }
    }
    for (int i=0 ; i < FLEX_SPI_FREQS_COUNT_ARRAY; i++)
    {
        if (FlexSpiFreqPodfFracDivArray[i].freq >= tmpFreq)
        {
            *freq = FlexSpiFreqPodfFracDivArray[i].freq;
            *flexSpiDiv = FlexSpiFreqPodfFracDivArray[i].flexSpiDiv;
            *pllSource = FlexSpiFreqPodfFracDivArray[i].pllSource;
            *aux0Div = FlexSpiFreqPodfFracDivArray[i].aux0div;
            *aux0pfd = FlexSpiFreqPodfFracDivArray[i].aux0pfd;
            foundFreq = true;
            break;
        }
    }
    if (foundFreq == false)
    {
        *freq = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY-1)].freq;
        *flexSpiDiv = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY-1)].flexSpiDiv;
        *pllSource = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].pllSource;
        *aux0Div = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].aux0div;
        *aux0pfd = FlexSpiFreqPodfFracDivArray[(FLEX_SPI_FREQS_COUNT_ARRAY -1)].aux0pfd;
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:     flexspi_clock_change                                                                      */
/*                                                                                                         */
/* Parameters:   base        - FLEXSPI_Type pointer for FLEXSPI instance                                   */
/*               freq        - SPI frequency value from SPI_FREQUENCY_T ENUM                               */
/*               dtrMode     - boolean parameter for DTR configuration                                     */
/*               definedFreq - return frequency reached by hardware                                        */
/* Returns:                                                                                                */
/* Side effects: none                                                                                      */
/* Description:  This function is used to change SPI frequency.                                            */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_clock_change(FLEXSPI_Type *base, uint32_t freq, bool dtrMode, uint32_t* definedFreq)
{
    uint32_t aux0pfd, aux0div, flexSpiDiv, pllSource;
    uint32_t currentFreq = freq;

    GetFlexSpiFreqDivAndPllSource_L(&currentFreq, dtrMode, &flexSpiDiv, &aux0pfd, &aux0div, &pllSource);
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

    if (pllSource == FLEXSPI_CLK_SEL_AUX0_PLL)
    {
        CLOCK_InitSysPfd(kCLOCK_Pfd2, aux0pfd);
        CLOCK_SetClkDiv(kCLOCK_DivAux0PllClk, aux0div);
    }

    BOARD_SetFlexspiClock(pllSource, flexSpiDiv);

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

    //PRINTF("FlexSPI Clock = %u MHz\n", currentFreq);

    if ((g_spi_freq != currentFreq) || (g_dtr_mode != dtrMode))
    {
        flexspi_enable_dqs(base, currentFreq, g_set_dqs_mode, dtrMode);
    }

    g_spi_freq = currentFreq;
    g_dtr_mode = dtrMode;

    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        FLEXSPI_IRQHandler                                                                     */
/*                                                                                                         */
/* Side effects:    none                                                                                   */
/* Description:     This function is used to handle FLEXSPI interrupt                                      */
/*                  The interrupt is generated when the SEQWAIT parameter is reached, meaning, the FlexSPI */
/*                  generated multiple clock output signals, but did not received expected DQS clock       */
/*                  signals input, this handler raises the flexspi_irq_flag flag and clears the interrupt  */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
void FLEXSPI_IRQHandler(void)
{
    uint32_t intr = FLEXSPI->INTR;

    // Check for SEQTIMEOUT
    if (intr & FLEXSPI_INTR_SEQTIMEOUT_MASK)
    {
        // Clear the interrupt flag
        FLEXSPI->INTR = FLEXSPI_INTR_SEQTIMEOUT_MASK;

        FLEXSPI->INTR |= FLEXSPI_INTR_IPCMDDONE_MASK;

        // Set software flag
        flexspi_irq_flag = true;
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        ConfigureFlexspiTimeout                                                                */
/*                                                                                                         */
/* Parameters:      base           - FLEXSPI_Type pointer for FLEXSPI instance                             */
/*                  timeoutOnOrOff - enable or disable FlexSpi timeout                                     */
/*                                                                                                         */
/* Side effects:    generate interrupt when SEQWAIT value have been reached before                         */
/*                  FLEXSPI transaction have finished                                                      */
/* Description:     This function is used to configure NXP FLEXSPI SEQWAIT parameter and enable interrupt  */
/*                  on SEQTIMEOUT                                                                          */
/* Note:            Mostly used in DQS mode, as read transaction are pending on external DQS signal,       */
/*                  when clock output signal count exceeds SEQWAIT * 1024 * Root Clock cycles,             */
/*                  FlexSPI interrupt with be generated and handled by FLEXSPI_IRQHandler function         */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int ConfigureFlexspiTimeout(FLEXSPI_Type *base, bool timeoutOnOrOff)
{
    if (timeoutOnOrOff == false)
    {
        base->INTEN &= ~FLEXSPI_INTEN_SEQTIMEOUTEN_MASK;
    }
    else
    {
        base->MCR1 &= ~FLEXSPI_MCR1_SEQWAIT_MASK;
        base->MCR1 |= FLEXSPI_MCR1_SEQWAIT(WAITSEQ_MAX_READ_1024_MULTIPLE); // Adjust this as needed

        // Enable SEQTIMEOUT interrupt
        base->INTEN |= FLEXSPI_INTEN_SEQTIMEOUTEN_MASK;

        // Enable IRQ in NVIC
        NVIC_EnableIRQ(FLEXSPI_IRQn);
    }
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
/*                  resetSpi  - boolean parameter to reset FlexSPI after re-configuration                  */
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

    /* DQS enabled, and freq above 100Mhz, use DLL lock and set dllValue for sample point       */
    /* DLL is lock on root clock, and in DTR, root clock = 2xFreq, therefore,                   */
    /* In DTR for 1/4T-clk = 1/2T-rootClk, and therefore, dllValue = 16 out of 32 quanta points */
    if (enableDqs)
    {
        if (((dtrMode == true) && ((freq * 2) >= SPI_FREQUENCY_100MHz)) ||
                (freq > SPI_FREQUENCY_100MHz))
        {
            // In DTR sample in 1/4T-clk 1/2T-rootClk=16, in SDR on same falling edge of DQS, value = 0.
            dllValue = (dtrMode == true) ? 16 : 0;
            useflexspiDllLockForDqs = true;
        }
    }

    if (dtrMode)
    {
        // Calculate the waveInNanoSeconds, meaning the time in [ns] from last dummy cycle clock edge,
        // from which the data output should be generated, to the first trigger point to sample the data,
        // In DTR the sample point is at the first raising edge, and in SDR, first falling edge
        // and also represent the wave length T[ns] for the Root clock used in DQS DLL lock.
        waveInNanoSeconds = (dtrMode) ? (500.0/(float)freq) : (1000.0/(float)freq);

        // When the DQS is not enable, the DLL lock is not used, and sampling point is calculated,
        // as quanta of dllValue 0-63 values, per quanta, 225ps delay.
        if (!enableDqs)
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
        // g_tdssq[ns], setup time from DQS edge to valid data
        // When the DQS is enabled, the sampling point is set as dllValue 0-31, per quanta percentage of T
        // for example, dllValue=16 1/2T-rootClk.
        // used to let the user set modified sampling point as cross-talk interference can effect performance,
        // in high operating frequencies.
        else if (g_tdssq < waveInNanoSeconds)
        {
            dllValue = (uint32_t)((g_tdssq / waveInNanoSeconds) * 32.0)+1;
            dllValue = (dllValue > 32) ? 32 : dllValue;
        }
    }
    if (dllValue > 0)
    {
        dllValue = ((dllValue-1) > 63) ? 63 : (dllValue-1);
    }

    if (useflexspiDllLockForDqs == false)
    {
        flexspiDllValue = FLEXSPI_DLLCR_OVRDEN(1) | FLEXSPI_DLLCR_DLLEN(0) | FLEXSPI_DLLCR_OVRDVAL(dllValue);
    }
    else
    {
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
        ConfigureFlexspiTimeout(base, true);
    }
    else
    {
        base->MCR0 |= FLEXSPI_MCR0_RXCLKSRC(FLEXSPI_RX_SAMPLE_CLOCK);
        g_dqs_mode = false;
        ConfigureFlexspiTimeout(base, false);
    }

    /* Exit stop mode. */
    base->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    /* Wait at least 100 NOPs*/
    for (uint32_t delay = 1000U; delay > 0U; delay--)
    {
        __NOP();
    }

    /* According to ERR011377, need to delay at least 100 NOPs to ensure the DLL is locked, for used PortA */
    statusValue = ((uint32_t)kFLEXSPI_FlashASampleClockSlaveDelayLocked | (uint32_t)kFLEXSPI_FlashASampleClockRefDelayLocked);

    /* Wait at least 100 NOPs*/
    for (uint32_t delay = 1000U; delay > 0U; delay--)
    {
        __NOP();
    }

    if (0U != (flexspiDllValue & FLEXSPI_DLLCR_DLLEN_MASK))
    {
        /* Wait at least 100 NOPs*/
        for (uint8_t delay = 100U; delay > 0U; delay--)
        {
            __NOP();
        }

        /* Wait slave delay line locked and slave reference delay line locked. */
        while ((base->STS2 & statusValue) != statusValue)
        {
            base->DLLCR[0] |= FLEXSPI_DLLCR_OVRDEN_MASK;
            base->DLLCR[0] &= ~FLEXSPI_DLLCR_OVRDEN_MASK;


        }
    }

    /* Wait for bus to be idle before continue. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

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
/* Returns:                                                                                                */
/* Side effects: none                                                                                      */
/* Description:  This function is used to initialize SPI frequency.                                        */
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int flexspi_clock_init(FLEXSPI_Type *base, uint32_t freq, bool enableDQS, bool dtrMode, uint32_t* definedFreq)
{
    flexspi_clock_change(base, freq, dtrMode, definedFreq);
    if (enableDQS)
    {
        flexspi_enable_dqs(base, freq, enableDQS, dtrMode);
    }

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

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_EnableDQS(bool enable)
{
    g_set_dqs_mode = enable;
    return flexspi_enable_dqs(FLEXSPI, g_spi_freq, g_set_dqs_mode, g_dtr_mode);
}

__attribute__ ((section(".ramfunc.$RAM")))
bool PLAT_SPI_GetDqsMode(void)
{
    return g_dqs_mode;
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_GetFreqInHz(uint32_t* freq)
{
    *freq = g_spi_freq * 1000000;
    return 0;
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_SetFreqInHz(uint32_t freq)
{
    g_set_spi_freq = (freq/1000000);
    return flexspi_clock_change(FLEXSPI, g_set_spi_freq, g_dtr_mode, NULL);
}

#define PLAT_QLIB_BUS_MODE(platBusMode) ((QLIB_BUS_MODE_T)platBusMode)


// Convert return value of NXP status to QLIB_STATUS
//#ifdef __QLIB_PLATFORM_H__ // TODO
__attribute__ ((section(".ramfunc.$RAM")))
__inline__ int NXP_writeReadTransaction(PLAT_SPI_FORMAT_T plat_format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize)
{
    uint32_t ints;
    uint32_t smallData = 0;
    flexspi_transfer_t flashXfer;
    status_t status;
    uint8_t  cmd1 = 0;
    uint8_t  cmd2 = 0;

    uint32_t address = 0;
    const uint8_t* dataOut = NULL;

    uint32_t lutSequence[LUT_DYNAMIC_SEQUENCE_SIZE] = {0};
    uint32_t InstrIndex = 0;
    uint32_t SequenceIndex = 0;
    uint32_t instList[8]; // cmd + addr + mode + dummy + (data in / data out) + stop

    uint32_t dtrCmdMask;
    uint32_t dtrAddrMask;
    uint32_t dtrDataMask;
    uint32_t padsCount[3];

    uint8_t SequenceToUse = LUT_SEQUENCE_TO_USE;

    uint32_t modeCmdType = 0;
    uint8_t  modeValue1 = 0;
    uint8_t  modeValue2 = 0;
    uint32_t modeBitsSize = 0;
#if !defined STRESS_FROM_HOST && defined ENABLE_SPI_COUNTERS
    g_SPI_Trns_Cnt++;
#endif

#ifdef WINBOND_DEBUG
    /*-----------------------------------------------------------------------------------------------------*/
    /* Error checking                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    ASSERT_RET(0 == dataInSize || 2 >= dataOutSize, -1);
    ASSERT_RET((cmdSize <= 2), -1);
    ASSERT_RET(addressSize == 0 || addressSize == 3 || addressSize == 4, -1);
#endif

    if ((dataInSize > 4) && (0 != (dataInSize % 4)))
    {
        dataInSize += 4 - (dataInSize % 4);
    }

    if (cmdSize == 2)
    {
        cmd1 = dataOutStream[0];
        cmd2 = dataOutStream[1];
    }
    else if (cmdSize != 0)
    {
        cmd1 = dataOutStream[0];
    }

#ifdef _STRESS_FROM_HOST
    // Read GET_SSR or RD_SR in low frequency, due to infinite while loop in QLIB layer,
    // to check if Device Under Test (DUT) Busy flag return to Ready.
    // After the GET_SSR or RD_SR is finished, returns to original frequency.
    if (g_reduce_freq_bypass_for_infinite_read_busy_flag)
    {
        if (g_set_spi_freq > g_reduce_freq_for_read_busy)
        {
            if (dataInSize > 0)
            {
                if ((cmd1 == 0xA0) || (cmd1 == 0xD0) || (cmd1 == 0xF0) || (cmd1 == 0x05))
                {
                    if (g_spi_freq != g_reduce_freq_for_read_busy)
                    {
                        flexspi_clock_change(FLEXSPI, g_reduce_freq_for_read_busy, g_dtr_mode, NULL);
                    }
                }
                else
                {
                    if (g_set_spi_freq != g_spi_freq)
                    {
                        flexspi_clock_change(FLEXSPI, g_set_spi_freq, g_dtr_mode, NULL);
                    }
                }
            }
            else
            {
                if (g_set_spi_freq != g_spi_freq)
                {
                    flexspi_clock_change(FLEXSPI, g_set_spi_freq, g_dtr_mode, NULL);
                }
            }
        }
    }
#endif


    if (addressSize == 4)
    {
        address = MAKE32B(dataOutStream[(cmdSize+3)], dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize]);
    }
    else if (addressSize == 3)
    {
        address = MAKE32B(dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize], 0);
    }

    // Backward compatibility and SSE support, use dataOutSize > 0 && dataInSize != 0, to indicate send data in mode type.
    if ((dataOutSize > 0) && (dataInSize != 0))
    {
        modeBitsSize = QLIB_SPI_FLAGS__GET_MODE_BITS_SIZE(flags);
        // keep Backward compatibility and SSE support, modeBits set multiple of 8 bits with dataOutSize
        modeBitsSize = (modeBitsSize > 0) ? modeBitsSize : (dataOutSize * 8);
    }

    if (dataOutSize > 0)
    {
        if (modeBitsSize > 0)
        {
            modeValue1 = dataOutStream[cmdSize + addressSize];
            if (modeBitsSize > 8)
            {
                modeValue2 = dataOutStream[cmdSize + addressSize + 1];
                dataOut = (dataOutStream + cmdSize + addressSize + 2);
            }
            else
            {
                dataOut = (dataOutStream + cmdSize + addressSize + 1);
            }
        }
        else
        {
            dataOut = (dataOutStream + cmdSize + addressSize);
        }
    }

    // FlexSPI frequency depends on SDR/DTR mode, therefore, any new command that change the SDR/DTR mode,
    // requires internal change of the mode, and the frequency accordingly. For example,
    // when the internal frequency is 332MHz (Max frequency), when sending DTR SPI command,
    // the output signals frequency generated, is automatically divided by 2, 166MHz (Max DTR frequency).
    if ((((flags & 0x7) != 0x0) ? true : false) != g_dtr_mode)
    {
        flexspi_clock_change(FLEXSPI, g_set_spi_freq, (SPI_CMD_DTR(flags) ? true : false), NULL);
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Disable interrupts                                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    ints = DisableGlobalIRQ();

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Can't replace MCU fetch command when direct access is enabled
    /*-----------------------------------------------------------------------------------------------------*/
    /* Check if command is already in LUT                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    QLIB_BUS_MODE_T format = PLAT_QLIB_BUS_MODE(plat_format);
    bool cmdDtrEnabled = (SPI_CMD_DTR(flags) ? true : false);
    if ((format != QLIB_BUS_MODE_1_1_1) && SPI_CMD_IS_SECURE(cmd1, format, flags) && ((format != g_LUT_format) || (format == QLIB_BUS_MODE_8_8_8 && cmdDtrEnabled != g_LUT_DTR)))
    {
        // Initialize the correct LUT
        flexspi_secure_LUT_init(format, cmdDtrEnabled);
    }
#if defined QLIB_SUPPORT_QPI || defined QLIB_SUPPORT_OPI
    if (!((((cmd1 == SPI_FLASH_CMD__READ_STATUS_REGISTER_1) || (cmd1 ==  SPI_FLASH_CMD__ENTER_QPI)) && !QLIB_BUS_MODE_IS_SPI(format)) || ((cmd1 == SPI_FLASH_CMD__ENTER_SPI) && (format != QLIB_BUS_MODE_4_4_4))))
#endif
    {
        SequenceToUse = g_SequenceTable[cmd1];

        if (LUT_SEQUENCE_TO_USE != SequenceToUse)
        {
            if ((SEC_CMD_OP1_SDR_SINGLE == cmd1 || SEC_CMD_OP1_SDR_QUAD == cmd1 || SEC_CMD_OP1_SDR_OCTAL == cmd1) && (dataOutSize > 0))
            {
                SequenceToUse += 1;
            }
            goto RUN_CMD;
        }
    }
#endif // QLIB_NO_DIRECT_FLASH_ACCESS
#endif // SPI_LUT_OPTIMIZATION_ENABLED

    /*-----------------------------------------------------------------------------------------------------*/
    /* Prepare the command sequence LUT                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
#ifndef STRESS_FROM_HOST
    g_LUT_Update_Cnt++;
#endif

    dtrCmdMask = (flags & QLIB_SPI_FLAGS__CMD_PHASE_DTR) ? 0x20 : 0x00;
    dtrAddrMask = (flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) ? 0x20 : 0x00;
    dtrDataMask = (flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) ? 0x20 : 0x00;

    switch (plat_format)
    {
        case PLAT_SPI_FORMAT_1_1_1:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_1PAD;
            break;
#ifdef QLIB_SUPPORT_DUAL_SPI
        case PLAT_SPI_FORMAT_1_1_2:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_2PAD;
            break;
        case PLAT_SPI_FORMAT_1_2_2:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_2PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_2PAD;
            break;
#endif // #endif QLIB_SUPPORT_DUAL_SPI

        case PLAT_SPI_FORMAT_1_1_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
        case PLAT_SPI_FORMAT_1_4_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
#ifdef QLIB_SUPPORT_QPI
        case PLAT_SPI_FORMAT_4_4_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
#endif // #endif QLIB_SUPPORT_QPI

        case PLAT_SPI_FORMAT_1_8_8:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_8PAD;
            break;
#ifdef QLIB_SUPPORT_OPI
        case PLAT_SPI_FORMAT_8_8_8:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_8PAD;
            break;
#endif // #endif QLIB_SUPPORT_OPI
        default:
            EnableGlobalIRQ(ints);
            return -1;
    }

    // 1: cmd
    if(0 != cmdSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_SDR | dtrCmdMask, padsCount[FLEXSPI_SPI_CMD_PHASE], cmd1);
        if (cmdSize == 2)
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_SDR | dtrCmdMask, padsCount[FLEXSPI_SPI_CMD_PHASE], cmd2);
        }
    }

    // 2: addr
    if (0 != addressSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_RADDR_SDR | dtrAddrMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], addressSize * 8);
    }

    // write
    if ((0 != dataOutSize) && !(dataOutSize != 0 && dataInSize != 0))
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_WRITE_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], 0); // no operands
    }

    // mode bits
    if (modeBitsSize > 0)
    {
        // Verify Mode Command type MODE8/4/2/1 is not lower then padsCount kFLEXSPI_8PAD/4PAD/2PAD/1PAD.
        // for example, if pad is set to kFLEXSPI_8PAD, mode type must be MODE8, and not lower (MODE4/2/1).
        // Mode kFLEXSPI_Command_MODE8/4/2/1 defines the amount of mode bits to send, and not the pad count.
        if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_8PAD) || (modeBitsSize == 8) || (modeBitsSize == 16))
        {
            modeCmdType = kFLEXSPI_Command_MODE8_SDR;
        }
        else if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_4PAD) || (modeBitsSize == 4))
        {
            modeCmdType = kFLEXSPI_Command_MODE4_SDR;
        }
        else if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_2PAD) || (modeBitsSize == 2))
        {
            modeCmdType = kFLEXSPI_Command_MODE2_SDR;
        }
        else
        {
            modeCmdType = kFLEXSPI_Command_MODE1_SDR;
        }

        if (modeBitsSize <= 8)
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);

            // when sending mode bits, dummy cycles count is reduced.
            // in DTR mode, if mode bits count is lower than data/dummy bus pin count, only rising edge is sent,
            // hence, need to duplicate mode bits data on falling edge.
            if (((flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) != 0) && (modeBitsSize >= (uint32_t)(1 << padsCount[FLEXSPI_SPI_ADDR_PHASE])))
            {
                instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                        modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);
            }
        }
        else
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                            modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                            modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue2);
        }
    }

    // dummy clocks
    if (0 != dummyCycles)
    {
        // In case of DTR mode, multiply dummy cycles number
        if ((flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) != 0)
        {
            dummyCycles = (2 * dummyCycles);
        }
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_DUMMY_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], dummyCycles);
    }

    // Read
    if (0 != dataInSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_READ_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], 0x04); // no operands
    }

    // Stop instruction
    instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_STOP, 0, 0);
//    if (((InstrIndex % 2) != 0) && (InstrIndex < 7))
//    {
//        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_STOP, 0, 0);
//    }

    for(SequenceIndex = 0; SequenceIndex < (InstrIndex / 2); SequenceIndex++)
    {
        lutSequence[SequenceIndex] = FLEXSPI_LUT_SEQ_MAKE_LINE(
                instList[SequenceIndex*2], instList[1 + SequenceIndex*2]);
    }

    // Update command LUT table
    FLEXSPI_UpdateLUT(FLEXSPI, LUT_DYNAMIC_SEQUENCE_INDEX * 4, lutSequence, LUT_DYNAMIC_SEQUENCE_SIZE);

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Unused tag
RUN_CMD:
#endif // QLIB_NO_DIRECT_FLASH_ACCESS
#endif // SPI_LUT_OPTIMIZATION_ENABLED
    /*-----------------------------------------------------------------------------------------------------*/
    /* Prepare command in structure                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = SequenceToUse;

    flashXfer.deviceAddress = (0 != addressSize) ? address : 0;

    if (0 != dataInSize)
    {
        flashXfer.cmdType = kFLEXSPI_Read;
        // Support data size smaller than 32bit using temp var, to prevent user buffer overflow
        flashXfer.data = (dataInSize < sizeof(uint32_t)) ? &smallData : (uint32_t*)dataIn;
        flashXfer.dataSize = dataInSize;
    }
    else
    {
        flashXfer.cmdType = (0 != dataOutSize) ? kFLEXSPI_Write : kFLEXSPI_Command;
        flashXfer.data =  (0 != dataOutSize) ? (uint32_t*)dataOut : 0;
        flashXfer.dataSize =  (0 != dataOutSize) ? dataOutSize : 0;
    }

#ifdef Q2_GPT_ACCU
    if(g_GPT_Accu)
    {
        GPT_StartTimer(GPT2);
    }
#endif

    // execute the command
    if (g_dqs_mode)
    {
        FLEXSPI_EnableInterrupts(FLEXSPI, (uint32_t)kFLEXSPI_SequenceExecutionTimeoutFlag);
    }

    status = NXP_FLEXSPI_TransferBlocking_L(FLEXSPI, &flashXfer);
#ifdef STRESS_FROM_HOST
    // use timeout timer, to stop infinite loop as timeout error
//    if (g_use_timeOut)
//    {
//        if (HiResIsReachedTimerTimeout())
//        {
//            return kStatus_Timeout;
//        }
//    }
    // use flexspi internal timeout interrupt handler which used in DQS mode to monitor
    // expected read count vs clock output, and on a failure return kStatus_Timeout
    if (g_reach_flexspi_timeout_interrupt_failure)
    {
        g_reach_flexspi_timeout_interrupt_failure = false;
        return kStatus_Timeout;
    }
#endif
#ifdef Q2_GPT_ACCU
    if(g_GPT_Accu)
    {
        GPT_StopTimer(GPT2);
    }
#endif

    if (status != kStatus_Success)
    {
        EnableGlobalIRQ(ints);
        return status;
    }

    if (flashXfer.cmdType == kFLEXSPI_Read)
    {
        if (0 != flashXfer.dataSize && flashXfer.dataSize < sizeof(uint32_t))
        {
#ifdef WINBOND_DEBUG
        // avoid using memcpy(dataIn, &smallData, dataInSize) since memcpy is located on flash
#endif
            dataIn[0] = ((uint8_t*)&smallData)[0];
            if (dataInSize > 1)
            {
                dataIn[1] = ((uint8_t*)&smallData)[1];
            }
            if (dataInSize > 2)
            {
                dataIn[2] = ((uint8_t*)&smallData)[2];
            }
        }
    }



      (ints);

    return 0;
}


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

void flexspi_handle_irq()
{
    // Optional recovery: reset FlexSPI
    FLEXSPI_SoftwareReset(FLEXSPI);

    /* Wait at least 100 NOPs*/
    for (uint32_t delay = 1000U; delay > 0U; delay--)
    {
        __NOP();
    }

    /* Wait for bus to be idle before continue. */
    while (!FLEXSPI_GetBusIdleStatus(FLEXSPI))
    {
    }

    flexspi_clock_change(FLEXSPI, 30, g_dtr_mode, NULL);
    flexspi_enable_dqs(FLEXSPI, 30, false, g_dtr_mode);

    g_reach_flexspi_timeout_interrupt_failure = true;
    flexspi_irq_flag = false;
}

// Modified version of NXP FLEXSPI_TransferBlocking function, that those the following additional tasks:
// 1. handle FlexSpi interrupt in infinite loop Wait until the IP command execution finishes.
// 2. for xfer->cmdType == kFLEXSPI_Read, also calls modified NXP_FLEXSPI_ReadBlocking_L function, that also
// handles the FlexSpi interrupt
// 3. As only this function is called for FlexSpi transactions, to improve performance, remove
// second call to while (!FLEXSPI_GetBusIdleStatus(base)) at the end of this function.
__attribute__ ((section(".ramfunc.$RAM")))
int NXP_FLEXSPI_TransferBlocking_L(FLEXSPI_Type *base, flexspi_transfer_t *xfer)
{
    uint32_t configValue = 0;
    status_t result      = kStatus_Success;

    /* Wait for bus to be idle before changing flash configuration. */
    while (!FLEXSPI_GetBusIdleStatus(base))
    {
    }

    /* Clear sequence pointer before sending data to external devices. */
    base->FLSHCR2[xfer->port] |= FLEXSPI_FLSHCR2_CLRINSTRPTR_MASK;

    /* Clear former pending status before start this transfer. */
    base->INTR = FLEXSPI_INTR_AHBCMDERR_MASK | FLEXSPI_INTR_IPCMDERR_MASK | FLEXSPI_INTR_AHBCMDGE_MASK |
                 FLEXSPI_INTR_IPCMDGE_MASK | FLEXSPI_INTR_IPCMDDONE_MASK;

    /* Configure base address. */
    base->IPCR0 = xfer->deviceAddress;

    /* Reset fifos. */
    base->IPTXFCR |= FLEXSPI_IPTXFCR_CLRIPTXF_MASK;
    base->IPRXFCR |= FLEXSPI_IPRXFCR_CLRIPRXF_MASK;

    /* Configure data size. */
    if ((xfer->cmdType == kFLEXSPI_Read) || (xfer->cmdType == kFLEXSPI_Write) || (xfer->cmdType == kFLEXSPI_Config))
    {
        configValue = FLEXSPI_IPCR1_IDATSZ(xfer->dataSize);
    }

    /* Configure sequence ID. */
    configValue |=
        FLEXSPI_IPCR1_ISEQID((uint32_t)xfer->seqIndex) | FLEXSPI_IPCR1_ISEQNUM((uint32_t)xfer->SeqNumber - 1U);
    base->IPCR1 = configValue;

    /* Start Transfer. */
    base->IPCMD |= FLEXSPI_IPCMD_TRG_MASK;

    if ((xfer->cmdType == kFLEXSPI_Write) || (xfer->cmdType == kFLEXSPI_Config))
    {
        result = FLEXSPI_WriteBlocking(base, (uint8_t *)xfer->data, xfer->dataSize);
    }
    else if (xfer->cmdType == kFLEXSPI_Read)
    {
        result = NXP_FLEXSPI_ReadBlocking_L(base, (uint8_t *)xfer->data, xfer->dataSize);
    }
    else
    {
        /* Empty else. */
    }

    /* Wait until the IP command execution finishes */
    while (0UL == (base->INTR & FLEXSPI_INTR_IPCMDDONE_MASK))
    {
        if (flexspi_irq_flag)
        {
            flexspi_handle_irq();
        }
    }

    /* Unless there is an error status already set, capture the latest one */
    if (result == kStatus_Success)
    {
        result = FLEXSPI_CheckAndClearError(base, base->INTR);
    }

    return result;
}

// Modified version of FLEXSPI_ReadBlocking, to also handle FlexSpi interrupt when using DQS.
__attribute__ ((section(".ramfunc.$RAM")))
int NXP_FLEXSPI_ReadBlocking_L(FLEXSPI_Type *base, uint8_t *buffer, size_t size)
{
    uint32_t rxWatermark = ((base->IPRXFCR & FLEXSPI_IPRXFCR_RXWMRK_MASK) >> FLEXSPI_IPRXFCR_RXWMRK_SHIFT) + 1U;
    uint32_t status;
    status_t result = kStatus_Success;
    uint32_t i      = 0;
    bool isReturn   = false;

    /* Send data buffer */
    while (0U != size)
    {
        if (size >= 8U * rxWatermark)
        {
            /* Wait until there is room in the fifo. This also checks for errors. */
            while (0U == ((status = base->INTR) & (uint32_t)kFLEXSPI_IpRxFifoWatermarkAvailableFlag))
            {
                if (flexspi_irq_flag)
                {
                    flexspi_handle_irq();
                }
                result = FLEXSPI_CheckAndClearError(base, status);

                if (kStatus_Success != result)
                {
                    isReturn = true;
                    break;
                }
            }
        }
        else
        {
            /* Wait fill level. This also checks for errors. */
            while (size > ((((base->IPRXFSTS) & FLEXSPI_IPRXFSTS_FILL_MASK) >> FLEXSPI_IPRXFSTS_FILL_SHIFT) * 8U))
            {
                result = FLEXSPI_CheckAndClearError(base, base->INTR);

                if (kStatus_Success != result)
                {
                    isReturn = true;
                    break;
                }
            }
        }

        if (isReturn)
        {
            break;
        }

        result = FLEXSPI_CheckAndClearError(base, base->INTR);

        if (kStatus_Success != result)
        {
            break;
        }

        /* Read watermark level data from rx fifo. */
        if (size >= 8U * rxWatermark)
        {
            for (i = 0U; i < 2U * rxWatermark; i++)
            {
                *(uint32_t *)(void *)buffer = base->RFDR[i];
                buffer += 4U;
            }

            size = size - 8U * rxWatermark;
        }
        else
        {
            /* Read word aligned data from rx fifo. */
            for (i = 0U; i < (size / 4U); i++)
            {
                *(uint32_t *)(void *)buffer = base->RFDR[i];
                buffer += 4U;
            }

            /* Adjust size by the amount processed. */
            size -= 4U * i;

            /* Read word un-aligned data from rx fifo. */
            if (0x00U != size)
            {
                uint32_t tempVal = base->RFDR[i];

                for (i = 0U; i < size; i++)
                {
                    *buffer++ = ((uint8_t)(tempVal >> (8U * i)) & 0xFFU);
                }
            }

            size = 0;
        }

        /* Pop out a watermark level datas from IP RX FIFO. */
        base->INTR = (uint32_t)kFLEXSPI_IpRxFifoWatermarkAvailableFlag;
    }

    return result;
}

int PLAT_SPI_WriteReadTransaction(const void*     userData,
                                  QLIB_BUS_MODE_T format,
                                  uint32_t        flags,
                                  const uint8_t*  dataOutStream,
                                  uint32_t        cmdSize,
                                  uint32_t        addressSize,
                                  uint32_t        dataOutSize,
                                  uint32_t        dummyCycles,
                                  uint8_t*        dataIn,
                                  uint32_t        dataInSize)
{
    return NXP_writeReadTransaction(format,
	                             flags,
	                             dataOutStream,
	                             cmdSize,
	                             addressSize,
	                             dataOutSize,
	                             dummyCycles,
	                             dataIn,
	                             dataInSize);
}
