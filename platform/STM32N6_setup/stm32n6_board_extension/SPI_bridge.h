/************************************************************************************************************
* @copyright  Copyright (c) 2026 by Winbond Electronics Corporation . All rights reserved
*
* @file       SPI_bridge.h
* @brief      This file contains header for SPI bridge of XSPI flash interface on STM32N6 device
*
************************************************************************************************************/

#ifndef _SPI_BRIDGE_H_
#define _SPI_BRIDGE_H_

#include "qlib_platform.h"
#include "stm32n6xx_hal.h"
#include "common_platform_spi.h"

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
    SPI_FREQUENCY_LAST=400
} SPI_FREQUENCY_T;

/*******************************************************************************
 * Declarations
 ******************************************************************************/

int STM32_enable_dqs(bool enable);
int STM32_writeReadTransaction(PLAT_SPI_FORMAT_T format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize);

#endif // _SPI_BRIDGE_H_
