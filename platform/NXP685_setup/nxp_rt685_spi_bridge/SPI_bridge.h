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

#ifndef _SPI_BRIDGE_H_
#define _SPI_BRIDGE_H_

#include "fsl_flexspi.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
#include "fsl_power.h"
#include "fsl_reset.h"

#include "common_platform_spi.h"
//#include "common_platform_capabilities.h"
//#include "common_platform_utils.h"
//#include "platform_revision.h"
//#include "flash_voltage.h"
//
//#include "HiResTimer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
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
    SPI_FREQUENCY_335MHz=335,
    SPI_FREQUENCY_340MHz=340,
    SPI_FREQUENCY_345MHz=345,
    SPI_FREQUENCY_350MHz=350,
    SPI_FREQUENCY_355MHz=355,
    SPI_FREQUENCY_360MHz=360,
    SPI_FREQUENCY_365MHz=365,
    SPI_FREQUENCY_370MHz=370,
    SPI_FREQUENCY_375MHz=375,
    SPI_FREQUENCY_380MHz=380,
    SPI_FREQUENCY_385MHz=385,
    SPI_FREQUENCY_390MHz=390,
    SPI_FREQUENCY_395MHz=395,
    SPI_FREQUENCY_400MHz=400,
    SPI_FREQUENCY_405MHz=405,
    SPI_FREQUENCY_410MHz=410,
    SPI_FREQUENCY_415MHz=415,
    SPI_FREQUENCY_420MHz=420,
    SPI_FREQUENCY_425MHz=425,
    SPI_FREQUENCY_430MHz=430,
    SPI_FREQUENCY_433MHz=433,
    SPI_FREQUENCY_435MHz=435,
    SPI_FREQUENCY_440MHz=440,
    SPI_FREQUENCY_445MHz=445,
    SPI_FREQUENCY_450MHz=450,
    SPI_FREQUENCY_455MHz=455,
    SPI_FREQUENCY_460MHz=460,
    SPI_FREQUENCY_465MHz=465,
    SPI_FREQUENCY_466MHz=466,
    SPI_FREQUENCY_470MHz=470,
    SPI_FREQUENCY_475MHz=475,
    SPI_FREQUENCY_480MHz=480,
    SPI_FREQUENCY_485MHz=485,
    SPI_FREQUENCY_490MHz=490,
    SPI_FREQUENCY_495MHz=495,
    SPI_FREQUENCY_500MHz=500,
    SPI_FREQUENCY_505MHz=505,
    SPI_FREQUENCY_510MHz=510,
    SPI_FREQUENCY_515MHz=515,
    SPI_FREQUENCY_520MHz=520,
    SPI_FREQUENCY_525MHz=525,
    SPI_FREQUENCY_530MHz=530,
    SPI_FREQUENCY_535MHz=535,
    SPI_FREQUENCY_540MHz=540,
    SPI_FREQUENCY_545MHz=545,
    SPI_FREQUENCY_550MHz=550,
    SPI_FREQUENCY_555MHz=555,
    SPI_FREQUENCY_560MHz=560,
    SPI_FREQUENCY_565MHz=565,
    SPI_FREQUENCY_570MHz=570,
    SPI_FREQUENCY_575MHz=575,
    SPI_FREQUENCY_580MHz=580,
    SPI_FREQUENCY_585MHz=585,
    SPI_FREQUENCY_590MHz=590,
    SPI_FREQUENCY_595MHz=595,
    SPI_FREQUENCY_600MHz=600,
    SPI_FREQUENCY_LAST=600
} SPI_FREQUENCY_T;

/*******************************************************************************
 * Declarations
 ******************************************************************************/
void flexspi_nor_flash_init(FLEXSPI_Type *base);
int flexspi_clock_change(FLEXSPI_Type *base, uint32_t freq, bool dtrMode, uint32_t* definedFreq);
int flexspi_enable_dqs(FLEXSPI_Type *base, uint32_t freq, bool enableDqs, bool dtrMode);
int flexspi_clock_init(FLEXSPI_Type *base, uint32_t freq, bool enableDQS, bool dtrMode, uint32_t* definedFreq);
int NXP_writeReadTransaction(PLAT_SPI_FORMAT_T format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize);
void flexspi_handle_irq(void);
void flexspi_set_tclqv(float tclqv);
void flexspi_set_tdssq(float tdssq);
#endif // _SPI_BRIDGE_H_
