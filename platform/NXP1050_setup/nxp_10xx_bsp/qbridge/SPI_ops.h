/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       SPI_ops.h
* @brief      This file includes qlib to nxp bridge definitions for SPI on NXP 10xx device
*
* ### project W77Q
*
************************************************************************************************************/

#ifndef __SPI_OPS_H__
#define __SPI_OPS_H__

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
// override QLIB multi-target
#ifndef QLIB_TARGET
#error "no QLIB_TARGET defined"
#endif
#include "qlib_targets.h"
#if (QLIB_TARGET == all_targets)
#undef QLIB_TARGET
#ifdef DEMO_FLASH_TARGET
#define QLIB_TARGET DEMO_FLASH_TARGET
#else
#error "DEMO_FLASH_TARGET must be defined with QLIB_TARGET == all_targets and SPI_LUT_OPTIMIZATION_ENABLED"
#endif
#endif
#endif

#include "fsl_clock.h"
#include "evkbimxrt1050_flexspi_nor_config.h"
#if !defined(QLIB_NO_DIRECT_FLASH_ACCESS)
#include "qlib.h"
#endif
#include "qlib_platform.h"
#include "defs.h"
#include "common_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LUT_SEQUENCE_TO_USE                          (15)

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
// NXP FlexSPI LUT for Q2 secure commands
#define W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP0                0
#define W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP0                1
#define W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1                2
#define W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP1_Data           3
#define W77Q_LUT_SEQ_IDX_1_1_1_SDR_OP2                4
#define W77Q_LUT_SEQ_IDX_1_1_1_DTR_OP2                5
#define W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP0              6
#define W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP0              7
#define W77Q_LUT_SEQ_IDX_DYNAMIC_OP1                  8
#define W77Q_LUT_SEQ_IDX_DYNAMIC_OP1_Data             9
#define W77Q_LUT_SEQ_IDX_DYNAMIC_SDR_OP2              10
#define W77Q_LUT_SEQ_IDX_DYNAMIC_DTR_OP2              11
#define W77Q_LUT_SEQ_IDX_1_1_1_READ_STATUS_REG1       12 // for Q2 BYPASS_HW_ISSUE_294
#define W77Q_LUT_SEQ_IDX_4_4_4_ENTER_SPI              13 // for Q2 MCD BYPASS_HW_ISSUE_23
#define W77Q_LUT_SEQ_IDX_1_1_1_ENTER_QPI              14 // for Q2 MCD BYPASS_HW_ISSUE_23
#define W77Q_LUT_SEQ_IDX_TEMP                         15

#define SEC_CMD_OP0_SDR_SINGLE  0xA0
#define SEC_CMD_OP0_SDR_DUAL    0xB0
#define SEC_CMD_OP0_SDR_QUAD    0xD0
#define SEC_CMD_OP0_SDR_OCTAL   0xF0
#define SEC_CMD_OP1_SDR_SINGLE  0xA1
//#define SEC_CMD_OP1_SDR_DUAL  0xB1
#define SEC_CMD_OP1_SDR_QUAD    0xD1
#define SEC_CMD_OP1_SDR_OCTAL   0xF1
#define SEC_CMD_OP2_SDR_SINGLE  0xA2
#define SEC_CMD_OP2_SDR_DUAL    0xB2
#define SEC_CMD_OP2_SDR_QUAD    0xD2
#define SEC_CMD_OP2_SDR_OCTAL   0xF2

#define SEC_CMD_OP0_DTR_SINGLE  0xA4
#define SEC_CMD_OP0_DTR_DUAL    0xB4
#define SEC_CMD_OP0_DTR_QUAD    0xD4
#define SEC_CMD_OP0_DTR_OCTAL   0xF0 // DOPI mode 8d-8d-8d
//#define SEC_CMD_OP1_DTR_SINGLE  0xA5
//#define SEC_CMD_OP1_DTR_DUAL    0xB5
//#define SEC_CMD_OP1_DTR_QUAD    0xD5
#define SEC_CMD_OP1_DTR_OCTAL   0xF1 // DOPI mode 8d-8d-8d
#define SEC_CMD_OP2_DTR_SINGLE  0xA6
#define SEC_CMD_OP2_DTR_DUAL    0xB6
#define SEC_CMD_OP2_DTR_QUAD    0xD6
#define SEC_CMD_OP2_DTR_OCTAL   0xF2 // DOPI mode 8d-8d-8d

#ifdef QLIB_FLASH_SIZE
#if QLIB_FLASH_SIZE <= _4MB_
#define SEC_CMD_OP0_SDR_DUMMY_CYCLES    32
#define SEC_CMD_OP0_DTR_DUMMY_CYCLES    16
#else
#define SEC_CMD_OP0_SDR_DUMMY_CYCLES    8
#define SEC_CMD_OP0_DTR_DUMMY_CYCLES    8
#endif
#else
#error missing QLIB_FLASH_SIZE definition
#endif

#define SEC_CMD_OP2_DUMMY_CYCLES        8

#define SEC_CMD_OP1_CTAG_SIZE           32
#endif // SPI_LUT_OPTIMIZATION_ENABLED

#define DIV_BY_1                        1
#define DIV_BY_2                        2
#define DIV_BY_4                        4
#define DIV_BY_6                        6
#define DIV_BY_7                        7
#define DIV_BY_8                        8
#define DQS_OFF                     false
#define DQS_ON                       true
#define DTR_OFF                     false
#define DTR_ON                       true

typedef enum SPI_FREQUENCY_T{
    SPI_FREQUENCY_FIRST=1,
    SPI_FREQUENCY_1MHz=1,
    SPI_FREQUENCY_2MHz=2,
    SPI_FREQUENCY_3MHz=3,
    SPI_FREQUENCY_4MHz=4,
    SPI_FREQUENCY_5MHz=5,
    SPI_FREQUENCY_10MHz=10,
    SPI_FREQUENCY_15MHz=15,
    SPI_FREQUENCY_20MHz=20,
    SPI_FREQUENCY_25MHz=25,
    SPI_FREQUENCY_30MHz=30,
    SPI_FREQUENCY_35MHz=35,
    SPI_FREQUENCY_40MHz=40,
    SPI_FREQUENCY_45MHz=45,
    SPI_FREQUENCY_50MHz=50,
    SPI_FREQUENCY_55MHz=55,
    SPI_FREQUENCY_60MHz=60,
    SPI_FREQUENCY_65MHz=65,
    SPI_FREQUENCY_70MHz=70,
    SPI_FREQUENCY_75MHz=75,
    SPI_FREQUENCY_80MHz=80,
    SPI_FREQUENCY_85MHz=85,
    SPI_FREQUENCY_90MHz=90,
    SPI_FREQUENCY_95MHz=95,
    SPI_FREQUENCY_100MHz=100,
    SPI_FREQUENCY_105MHz=105,
    SPI_FREQUENCY_110MHz=110,
    SPI_FREQUENCY_115MHz=115,
    SPI_FREQUENCY_120MHz=120,
    SPI_FREQUENCY_125MHz=125,
    SPI_FREQUENCY_130MHz=130,
    SPI_FREQUENCY_133MHz=133,
    SPI_FREQUENCY_135MHz=135,
    SPI_FREQUENCY_140MHz=140,
    SPI_FREQUENCY_145MHz=145,
    SPI_FREQUENCY_150MHz=150,
    SPI_FREQUENCY_155MHz=155,
    SPI_FREQUENCY_160MHz=160,
    SPI_FREQUENCY_165MHz=165,
    SPI_FREQUENCY_166MHz=166,
    SPI_FREQUENCY_170MHz=170,
    SPI_FREQUENCY_175MHz=175,
    SPI_FREQUENCY_180MHz=180,
    SPI_FREQUENCY_185MHz=185,
    SPI_FREQUENCY_190MHz=190,
    SPI_FREQUENCY_195MHz=195,
    SPI_FREQUENCY_200MHz=200,
    SPI_FREQUENCY_205MHz=205,
    SPI_FREQUENCY_210MHz=210,
    SPI_FREQUENCY_215MHz=215,
    SPI_FREQUENCY_220MHz=220,
    SPI_FREQUENCY_225MHz=225,
    SPI_FREQUENCY_230MHz=230,
    SPI_FREQUENCY_233MHz=233,
    SPI_FREQUENCY_235MHz=235,
    SPI_FREQUENCY_240MHz=240,
    SPI_FREQUENCY_245MHz=245,
    SPI_FREQUENCY_250MHz=250,
    SPI_FREQUENCY_255MHz=255,
    SPI_FREQUENCY_260MHz=260,
    SPI_FREQUENCY_265MHz=265,
    SPI_FREQUENCY_266MHz=266,
    SPI_FREQUENCY_270MHz=270,
    SPI_FREQUENCY_275MHz=275,
    SPI_FREQUENCY_280MHz=280,
    SPI_FREQUENCY_285MHz=285,
    SPI_FREQUENCY_290MHz=290,
    SPI_FREQUENCY_295MHz=295,
    SPI_FREQUENCY_300MHz=300,
    SPI_FREQUENCY_305MHz=305,
    SPI_FREQUENCY_310MHz=310,
    SPI_FREQUENCY_315MHz=315,
    SPI_FREQUENCY_320MHz=320,
    SPI_FREQUENCY_325MHz=325,
    SPI_FREQUENCY_330MHz=330,
    SPI_FREQUENCY_332MHz=332,
    SPI_FREQUENCY_LAST=332
} SPI_FREQUENCY_T;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int flexspi_nor_flash_init(FLEXSPI_Type *base);
int flexspi_clock_init(FLEXSPI_Type *base, uint32_t freq, bool enableDQS, bool dtrMode, uint32_t* definedFreq, bool toPrint);
int flexspi_clock_change(FLEXSPI_Type *base, uint32_t freq, bool dtrMode, uint32_t* definedFreq, bool toPrint);
int flexspi_enable_dqs(FLEXSPI_Type *base, uint32_t freq, bool enableDqs, bool dtrMode);
void flexspi_set_tclqv(float tclqv);
void flexspi_set_tdssq(float tdssq);
#ifdef SPI_LUT_OPTIMIZATION_ENABLED
status_t flexspi_secure_LUT_init(QLIB_BUS_MODE_T format, bool dtrMode);
#endif

uint32_t flexspi_getfreq(void);
bool flexspi_getDqsMode(void);

void flexspi_default_LUT_init(void);

uint32_t flexspi_get_fetch_cmd_lut_idx(void);

#endif /* __SPI_OPS_H__ */
