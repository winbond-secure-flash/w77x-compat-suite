/*---------------------------------------------------------------------------------------------------------*/
/* Winbond Electronics Corporation                                                                         */
/*                                                                                                         */
/*                                                                                                         */
/* Copyright (c) 2019 by Winbond Electronics Corporation                                                   */
/* All rights reserved                                                                                     */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
/* File Content:                                                                                           */
/*            This file contains XSPI flash interface implementation for STM32H7 MCU                       */
/* Project:                                                                                                */
/*            All STM32H7 projects                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <string.h>
#include "SPI_bridge.h"
#include "qlib_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/* Make 16-bit value from 2 byte values                                                                    */
/*                                                                                                         */
/* MAKE16 operation example:                                                                               */
/* Two 8bit sources: <byte1>, <byte2>                                                                      */
/* 16bit result:     <byte2><byte1>                                                                        */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define MAKE16(blo, bhi) ((uint16_t)(((uint16_t)(blo)) | (((uint16_t)(bhi)) << 8)))

/*---------------------------------------------------------------------------------------------------------*/
/* Make 32-bit value from 2 word values                                                                    */
/*                                                                                                         */
/* MAKE32 operation example:                                                                               */
/* Two 16bit sources: <word1>, <word2>                                                                     */
/* 32bit result:      <word2><word1>                                                                       */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define MAKE32(wlo, whi) ((uint32_t)(((uint32_t)(wlo)) | (((uint32_t)(whi)) << 16)))

/*---------------------------------------------------------------------------------------------------------*/
/* Make 32-bit value from 4 bytes                                                                          */
/*                                                                                                         */
/* MAKE32B operation example:                                                                              */
/* Four 8bit sources: <byte0>, <byte1>, <byte2>, <byte3>                                                   */
/* 32bit result:      <byte3><byte2><byte1><byte0>                                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define MAKE32B(b0, b1, b2, b3) MAKE32(MAKE16(b0, b1), MAKE16(b2, b3))

#define FREQ_100_MHZ   (100000000)
#define FREQ_200_MHZ   (200000000)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/


typedef struct
{
    uint32_t PLLN;
    uint32_t PLLT;
    uint32_t PLLFractional;
    uint32_t ClockPrescaler;
}XSPI_FREQUENCY_PARAMS_T;

/*******************************************************************************
 * Variables
 ******************************************************************************/

// TODO: find another solution without extern
extern XSPI_HandleTypeDef hxspi2;
XSPI_HandleTypeDef *hxspi = &hxspi2;

int g_dqs_mode = HAL_XSPI_DQS_DISABLE;

uint32_t g_spi_freq = FREQ_100_MHZ;  // STM32 initialized XSPI flash frequency


// Frequency params 0-400MHz (min freq = 10MHz)
XSPI_FREQUENCY_PARAMS_T g_xspi_frequency_table[401] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {25, 5, 0, 7},
    {27, 5, 4096, 7},
    {30, 5, 0, 7},
    {27, 8, 0, 3},
    {28, 8, 0, 3},
    {26, 7, 2048, 3},
    {28, 7, 0, 3},
    {25, 6, 4096, 3},
    {27, 6, 0, 3},
    {38, 8, 0, 3},
    {25, 5, 0, 3},
    {26, 5, 2048, 3},
    {27, 5, 4096, 3},
    {28, 5, 6144, 3},
    {30, 5, 0, 3},
    {25, 8, 0, 1},
    {26, 8, 0, 1},
    {27, 8, 0, 1},
    {28, 8, 0, 1},
    {25, 7, 3072, 1},
    {26, 7, 2048, 1},
    {31, 8, 0, 1},
    {28, 7, 0, 1},
    {33, 8, 0, 1},
    {25, 6, 4096, 1},
    {26, 6, 2048, 1},
    {27, 6, 0, 1},
    {27, 6, 6144, 1},
    {38, 8, 0, 1},
    {29, 6, 2048, 1},
    {25, 5, 0, 1},
    {25, 5, 5120, 1},
    {26, 5, 2048, 1},
    {26, 5, 7168, 1},
    {27, 5, 4096, 1},
    {28, 5, 1024, 1},
    {28, 5, 6144, 1},
    {29, 5, 2560, 1},
    {30, 5, 0, 1},
    {30, 5, 5120, 1},
    {25, 8, 0, 0},
    {25, 8, 4096, 0},
    {26, 8, 0, 0},
    {26, 8, 4096, 0},
    {27, 8, 0, 0},
    {27, 8, 4096, 0},
    {28, 8, 0, 0},
    {28, 8, 4096, 0},
    {25, 7, 3072, 0},
    {25, 7, 6656, 0},
    {26, 7, 2048, 0},
    {26, 7, 5632, 0},
    {31, 8, 0, 0},
    {31, 8, 4096, 0},
    {28, 7, 0, 0},
    {32, 8, 4096, 0},
    {33, 8, 0, 0},
    {25, 6, 1024, 0},
    {25, 6, 4096, 0},
    {25, 6, 7168, 0},
    {26, 6, 2048, 0},
    {26, 6, 5120, 0},
    {27, 6, 0, 0},
    {27, 6, 3072, 0},
    {27, 6, 6144, 0},
    {37, 8, 4096, 0},
    {38, 8, 0, 0},
    {38, 8, 4096, 0},
    {29, 6, 2048, 0},
    {29, 6, 5120, 0},
    {25, 5, 0, 0},
    {25, 5, 2560, 0},
    {25, 5, 5120, 0},
    {25, 5, 7680, 0},
    {26, 5, 2048, 0},
    {26, 5, 4608, 0},
    {26, 5, 7168, 0},
    {27, 5, 1536, 0},
    {27, 5, 4096, 0},
    {27, 5, 6656, 0},
    {28, 5, 1024, 0},
    {28, 5, 3584, 0},
    {28, 5, 6144, 0},
    {29, 5, 512, 0},
    {29, 5, 2560, 0},
    {29, 5, 5632, 0},
    {30, 5, 0, 0},
    {30, 5, 2560, 0},
    {30, 5, 5120, 0},
    {30, 5, 7680, 0},
    {25, 4, 0, 0},
    {25, 4, 2048, 0},
    {25, 4, 4096, 0},
    {25, 4, 6144, 0},
    {26, 4, 0, 0},
    {26, 4, 2048, 0},
    {26, 4, 4096, 0},
    {26, 4, 6144, 0},
    {27, 4, 0, 0},
    {27, 4, 2048, 0},
    {27, 4, 4096, 0},
    {27, 4, 6144, 0},
    {28, 4, 0, 0},
    {28, 4, 2048, 0},
    {28, 4, 4096, 0},
    {28, 4, 6144, 0},
    {29, 4, 0, 0},
    {29, 4, 2048, 0},
    {29, 4, 4096, 0},
    {29, 4, 6144, 0},
    {30, 4, 0, 0},
    {30, 4, 2048, 0},
    {30, 4, 4096, 0},
    {30, 4, 6144, 0},
    {31, 4, 0, 0},
    {31, 4, 2048, 0},
    {31, 4, 4096, 0},
    {31, 4, 6144, 0},
    {32, 4, 0, 0},
    {32, 4, 2048, 0},
    {32, 4, 4096, 0},
    {32, 4, 6144, 0},
    {33, 4, 0, 0},
    {33, 4, 2048, 0},
    {25, 3, 1024, 0},
    {25, 3, 2560, 0},
    {25, 3, 4096, 0},
    {25, 3, 5632, 0},
    {25, 3, 7168, 0},
    {26, 3, 512, 0},
    {26, 3, 2048, 0},
    {26, 3, 3584, 0},
    {26, 3, 5120, 0},
    {26, 3, 6656, 0},
    {27, 3, 0, 0},
    {27, 3, 1536, 0},
    {27, 3, 3072, 0},
    {27, 3, 4608, 0},
    {27, 3, 6144, 0},
    {27, 3, 7680, 0},
    {37, 4, 4096, 0},
    {37, 4, 6144, 0},
    {38, 4, 0, 0},
    {38, 4, 2048, 0},
    {38, 4, 4096, 0},
    {29, 3, 512, 0},
    {29, 3, 2048, 0},
    {29, 3, 3584, 0},
    {29, 3, 5120, 0},
    {29, 3, 6656, 0},
    {30, 3, 0, 0},
    {30, 3, 1536, 0},
    {30, 3, 3072, 0},
    {30, 3, 4608, 0},
    {30, 3, 6144, 0},
    {30, 3, 7680, 0},
    {41, 4, 4096, 0},
    {41, 4, 6144, 0},
    {42, 4, 0, 0},
    {42, 4, 2048, 0},
    {42, 4, 4096, 0},
    {32, 3, 512, 0},
    {32, 3, 2048, 0},
    {32, 3, 3584, 0},
    {32, 3, 5120, 0},
    {32, 3, 6656, 0},
    {33, 3, 0, 0},
    {33, 3, 1536, 0},
    {33, 3, 3072, 0},
    {33, 3, 4608, 0},
    {33, 3, 6144, 0},
    {33, 3, 7680, 0},
    {45, 4, 4096, 0},
    {45, 4, 6144, 0},
    {46, 4, 0, 0},
    {46, 4, 2048, 0},
    {46, 4, 4096, 0},
    {35, 3, 512, 0},
    {35, 3, 2048, 0},
    {35, 3, 3584, 0},
    {35, 3, 5120, 0},
    {35, 3, 6656, 0},
    {36, 3, 0, 0},
    {36, 3, 1536, 0},
    {36, 3, 3072, 0},
    {36, 3, 4608, 0},
    {36, 3, 6144, 0},
    {36, 3, 7680, 0},
    {49, 4, 4096, 0},
    {49, 4, 6144, 0},
    {25, 2, 0, 0},
    {25, 2, 1024, 0},
    {25, 2, 2048, 0},
    {25, 2, 3072, 0},
    {25, 2, 4096, 0},
    {25, 2, 5120, 0},
    {25, 2, 6144, 0},
    {25, 2, 7168, 0},
    {26, 2, 0, 0},
    {26, 2, 1024, 0},
    {26, 2, 2048, 0},
    {26, 2, 3072, 0},
    {26, 2, 4096, 0},
    {26, 2, 5120, 0},
    {26, 2, 6144, 0},
    {26, 2, 7168, 0},
    {27, 2, 0, 0},
    {27, 2, 1024, 0},
    {27, 2, 2048, 0},
    {27, 2, 3072, 0},
    {27, 2, 4096, 0},
    {27, 2, 5120, 0},
    {27, 2, 6144, 0},
    {27, 2, 7168, 0},
    {28, 2, 0, 0},
    {28, 2, 1024, 0},
    {28, 2, 2048, 0},
    {28, 2, 3072, 0},
    {28, 2, 4096, 0},
    {28, 2, 5120, 0},
    {28, 2, 6144, 0},
    {28, 2, 7168, 0},
    {29, 2, 0, 0},
    {29, 2, 1024, 0},
    {29, 2, 2048, 0},
    {29, 2, 3072, 0},
    {29, 2, 4096, 0},
    {29, 2, 5120, 0},
    {29, 2, 6144, 0},
    {29, 2, 7168, 0},
    {30, 2, 0, 0},
    {30, 2, 1024, 0},
    {30, 2, 2048, 0},
    {30, 2, 3072, 0},
    {30, 2, 4096, 0},
    {30, 2, 5120, 0},
    {30, 2, 6144, 0},
    {30, 2, 7168, 0},
    {31, 2, 0, 0},
    {31, 2, 1024, 0},
    {31, 2, 2048, 0},
    {31, 2, 3072, 0},
    {31, 2, 4096, 0},
    {31, 2, 5120, 0},
    {31, 2, 6144, 0},
    {31, 2, 7168, 0},
    {32, 2, 0, 0},
    {32, 2, 1024, 0},
    {32, 2, 2048, 0},
    {32, 2, 3072, 0},
    {32, 2, 4096, 0},
    {32, 2, 5120, 0},
    {32, 2, 6144, 0},
    {32, 2, 7168, 0},
    {33, 2, 0, 0},
    {33, 2, 1024, 0},
    {33, 2, 2048, 0},
    {33, 2, 3072, 0},
    {33, 2, 4096, 0},
    {33, 2, 5120, 0},
    {33, 2, 6144, 0},
    {33, 2, 7168, 0},
    {34, 2, 0, 0},
    {34, 2, 1024, 0},
    {34, 2, 2048, 0},
    {34, 2, 3072, 0},
    {34, 2, 4096, 0},
    {34, 2, 5120, 0},
    {34, 2, 6144, 0},
    {34, 2, 7168, 0},
    {35, 2, 0, 0},
    {35, 2, 1024, 0},
    {35, 2, 2048, 0},
    {35, 2, 3072, 0},
    {35, 2, 4096, 0},
    {35, 2, 5120, 0},
    {35, 2, 6144, 0},
    {35, 2, 7168, 0},
    {36, 2, 0, 0},
    {36, 2, 1024, 0},
    {36, 2, 2048, 0},
    {36, 2, 3072, 0},
    {36, 2, 4096, 0},
    {36, 2, 5120, 0},
    {36, 2, 6144, 0},
    {36, 2, 7168, 0},
    {37, 2, 0, 0},
    {37, 2, 1024, 0},
    {37, 2, 2048, 0},
    {37, 2, 3072, 0},
    {37, 2, 4096, 0},
    {37, 2, 5120, 0},
    {37, 2, 6144, 0},
    {37, 2, 7168, 0},
    {38, 2, 0, 0},
    {38, 2, 1024, 0},
    {38, 2, 2048, 0},
    {38, 2, 3072, 0},
    {38, 2, 4096, 0},
    {38, 2, 5120, 0},
    {38, 2, 6144, 0},
    {38, 2, 7168, 0},
    {39, 2, 0, 0},
    {39, 2, 1024, 0},
    {39, 2, 2048, 0},
    {39, 2, 3072, 0},
    {39, 2, 4096, 0},
    {39, 2, 5120, 0},
    {39, 2, 6144, 0},
    {39, 2, 7168, 0},
    {40, 2, 0, 0},
    {40, 2, 1024, 0},
    {40, 2, 2048, 0},
    {40, 2, 3072, 0},
    {40, 2, 4096, 0},
    {40, 2, 5120, 0},
    {40, 2, 6144, 0},
    {40, 2, 7168, 0},
    {41, 2, 0, 0},
    {41, 2, 1024, 0},
    {41, 2, 2048, 0},
    {41, 2, 3072, 0},
    {41, 2, 4096, 0},
    {41, 2, 5120, 0},
    {41, 2, 6144, 0},
    {41, 2, 7168, 0},
    {42, 2, 0, 0},
    {42, 2, 1024, 0},
    {42, 2, 2048, 0},
    {42, 2, 3072, 0},
    {42, 2, 4096, 0},
    {42, 2, 5120, 0},
    {42, 2, 6144, 0},
    {42, 2, 7168, 0},
    {43, 2, 0, 0},
    {43, 2, 1024, 0},
    {43, 2, 2048, 0},
    {43, 2, 3072, 0},
    {43, 2, 4096, 0},
    {43, 2, 5120, 0},
    {43, 2, 6144, 0},
    {43, 2, 7168, 0},
    {44, 2, 0, 0},
    {44, 2, 1024, 0},
    {44, 2, 2048, 0},
    {44, 2, 3072, 0},
    {44, 2, 4096, 0},
    {44, 2, 5120, 0},
    {44, 2, 6144, 0},
    {44, 2, 7168, 0},
    {45, 2, 0, 0},
    {45, 2, 1024, 0},
    {45, 2, 2048, 0},
    {45, 2, 3072, 0},
    {45, 2, 4096, 0},
    {45, 2, 5120, 0},
    {45, 2, 6144, 0},
    {45, 2, 7168, 0},
    {46, 2, 0, 0},
    {46, 2, 1024, 0},
    {46, 2, 2048, 0},
    {46, 2, 3072, 0},
    {46, 2, 4096, 0},
    {46, 2, 5120, 0},
    {46, 2, 6144, 0},
    {46, 2, 7168, 0},
    {47, 2, 0, 0},
    {47, 2, 1024, 0},
    {47, 2, 2048, 0},
    {47, 2, 3072, 0},
    {47, 2, 4096, 0},
    {47, 2, 5120, 0},
    {47, 2, 6144, 0},
    {47, 2, 7168, 0},
    {48, 2, 0, 0},
    {48, 2, 1024, 0},
    {48, 2, 2048, 0},
    {48, 2, 3072, 0},
    {48, 2, 4096, 0},
    {48, 2, 5120, 0},
    {48, 2, 6144, 0},
    {48, 2, 7168, 0},
    {49, 2, 0, 0},
    {49, 2, 1024, 0},
    {49, 2, 2048, 0},
    {49, 2, 3072, 0},
    {49, 2, 4096, 0},
    {49, 2, 5120, 0},
    {49, 2, 6144, 0},
    {49, 2, 7168, 0},
    {25, 1, 0, 0}
};







/*******************************************************************************
 * Code
 ******************************************************************************/


/**
  * @brief  Wait for a flag state until timeout.
  * @param  hxspi     : XSPI handle
  * @param  Flag      : Flag checked
  * @param  State     : Value of the flag expected
  * @param  Timeout   : Duration of the timeout
  * @param  Tickstart : Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef XSPI_WaitFlagStateUntilTimeout(XSPI_HandleTypeDef *hxspi, uint32_t Flag,
                                                        FlagStatus State, uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is in expected state */
  while ((HAL_XSPI_GET_FLAG(hxspi, Flag)) != State)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hxspi->State     = HAL_XSPI_STATE_READY;
        hxspi->ErrorCode |= HAL_XSPI_ERROR_TIMEOUT;

        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}

int PLAT_SPI_EnableDQS(bool enable)
{
    return STM32_enable_dqs(enable);
}

bool PLAT_SPI_GetDqsMode(void)
{
    return g_dqs_mode;
}

// XSPI2 Set Frequency [MHz]
// Parameters:
//   frequencyMhz Range: 10-400
// Description:
//   Configures PLL2 to generate the desired XSPI2 frequency
//   XSPI2 uses PLL2_T output: VCO_freq / PLLT / (ClockPrescaler + 1)
//   VCO frequency = (HSI / PLLM) * PLLN + PLLFractional
//   where HSI = 64 MHz, PLLM = 4 (16 MHz input to VCO)
// Return: 0 = PASS, otherwise return negative value
int XSPI_SetFrequency(XSPI_HandleTypeDef *hxspi, uint32_t frequencyMhz)
{
    uint32_t tickstart = HAL_GetTick();
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /* Validate frequency range */
    if (frequencyMhz < 10 || frequencyMhz > 400)
    {
        return -1;
    }

    /* Configure PLL2 oscillator parameters */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL2.PLLM = 4;
    RCC_OscInitStruct.PLL2.PLLN = g_xspi_frequency_table[frequencyMhz].PLLN;
    RCC_OscInitStruct.PLL2.PLLP = 2;
    RCC_OscInitStruct.PLL2.PLLQ = 2;
    RCC_OscInitStruct.PLL2.PLLR = 2;
    RCC_OscInitStruct.PLL2.PLLS = 1;
    RCC_OscInitStruct.PLL2.PLLT = g_xspi_frequency_table[frequencyMhz].PLLT;
    RCC_OscInitStruct.PLL2.PLLFractional = g_xspi_frequency_table[frequencyMhz].PLLFractional;
    RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return -1;
    }

    /* Enable PLL2_T clock output for XSPI2 */
    __HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL_TCLK);

    /* Configure XSPI2 peripheral clock source to use PLL2_T */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_XSPI2;
    PeriphClkInit.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_PLL2T;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        return -1;
    }

    /* Wait till busy flag is reset before changing prescaler */
    XSPI_WaitFlagStateUntilTimeout(hxspi, HAL_XSPI_FLAG_BUSY, RESET, tickstart, hxspi->Timeout);

    /* Configure XSPI2 clock prescaler
     * Final frequency = PLL2_T / (ClockPrescaler + 1)
     */
    MODIFY_REG(hxspi->Instance->DCR2, XSPI_DCR2_PRESCALER,
               ((g_xspi_frequency_table[frequencyMhz].ClockPrescaler) << XSPI_DCR2_PRESCALER_Pos));

    /* The configuration of clock prescaler triggers automatically a calibration process.
     * Wait for the calibration to complete */
    XSPI_WaitFlagStateUntilTimeout(hxspi, HAL_XSPI_FLAG_BUSY, RESET, tickstart, hxspi->Timeout);

    return 0;
}

int PLAT_SPI_GetFreqInHz(uint32_t* freq)
{
    *freq = g_spi_freq * 1000000;
    return 0;
}

int PLAT_SPI_SetFreqInHz(uint32_t freq)
{
    g_spi_freq = (freq/1000000);
    return XSPI_SetFrequency(&hxspi2, g_spi_freq);
}

int PLAT_GetCapabilities(PLAT_SPI_HW_CAPABILITIES_T* hwCapabilities)
{
    memset(hwCapabilities, 0, sizeof(PLAT_SPI_HW_CAPABILITIES_T));
    hwCapabilities->dqs = true;
    hwCapabilities->dtr = true;
    hwCapabilities->hw_flash_voltage_set = false;
    hwCapabilities->hw_gpio = false;
    hwCapabilities->hw_leds = true;
    hwCapabilities->hw_reset_to_flash = false;
    hwCapabilities->reset_out_from_flash = false;
    hwCapabilities->hw_sha = false;  // until will be implemented
    hwCapabilities->hw_int_input = false;
    hwCapabilities->hw_test_mode = false;
    hwCapabilities->hw_freq_set = false;
    hwCapabilities->hw_spi_sdr_min_freq_in_hz = 1000000;
    hwCapabilities->hw_spi_sdr_max_freq_in_hz = FREQ_100_MHZ;
    hwCapabilities->hw_spi_dtr_min_freq_in_hz = 2000000;
    hwCapabilities->hw_spi_dtr_max_freq_in_hz = FREQ_200_MHZ;
    hwCapabilities->hw_spi_max_read_buffer_size = 256;
    hwCapabilities->busModes.plat_spi_1_1_1 = true;
    hwCapabilities->busModes.plat_spi_1_1_2 = true;
    hwCapabilities->busModes.plat_spi_1_2_2 = true;
    hwCapabilities->busModes.plat_spi_1_1_4 = true;
    hwCapabilities->busModes.plat_spi_1_4_4 = true;
    hwCapabilities->busModes.plat_spi_4_4_4 = true;
    hwCapabilities->busModes.plat_spi_1_8_8 = true;
    hwCapabilities->busModes.plat_spi_8_8_8 = true;
    hwCapabilities->hw_test_mode = false;
    hwCapabilities->hw_timer = false; // doesn't support SPI transaction timeout timer
    return 0;
}

int STM32_enable_dqs(bool enable)
{
    if (enable)
        g_dqs_mode = HAL_XSPI_DQS_ENABLE;
    else
        g_dqs_mode = HAL_XSPI_DQS_DISABLE;
    return 0;
}

__inline__ int STM32_writeReadTransaction(PLAT_SPI_FORMAT_T plat_format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize)
{


    XSPI_RegularCmdTypeDef  sCommand = {0};

    sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE; // No alternate bytes phase - TODO: update when required write and read on same transaction

    // Check command
    if (cmdSize == 2)
    {
        sCommand.Instruction = *(uint16_t*)dataOutStream;
        sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    }
    else if (cmdSize == 1)
    {
        sCommand.Instruction = *(uint8_t*)dataOutStream;
        sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    }

    // Check DTR in command
    if (flags)
    {
        sCommand.InstructionDTRMode = (flags & QLIB_SPI_FLAGS__CMD_PHASE_DTR) ?  HAL_XSPI_INSTRUCTION_DTR_ENABLE : 0x00;
        sCommand.AddressDTRMode     = (flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) ? HAL_XSPI_ADDRESS_DTR_ENABLE     : 0x00;
        sCommand.DataDTRMode        = (flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) ? HAL_XSPI_DATA_DTR_ENABLE        : 0x00;
    }

    // Check command SPI lines
    switch (plat_format)
    {
        case PLAT_SPI_FORMAT_1_1_1:
            sCommand.InstructionMode = cmdSize ? HAL_XSPI_INSTRUCTION_1_LINE : HAL_XSPI_INSTRUCTION_NONE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_1_LINE;
            break;
#ifdef QLIB_SUPPORT_DUAL_SPI
        case PLAT_SPI_FORMAT_1_1_2:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_2_LINE;
            break;
        case PLAT_SPI_FORMAT_1_2_2:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_2_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_2_LINE;
            break;
#endif // #endif QLIB_SUPPORT_DUAL_SPI

        case PLAT_SPI_FORMAT_1_1_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
        case PLAT_SPI_FORMAT_1_4_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_4_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
#ifdef QLIB_SUPPORT_QPI
        case PLAT_SPI_FORMAT_4_4_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_4_LINES;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_4_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
#endif

        case PLAT_SPI_FORMAT_1_8_8:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_8_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_8_LINES;
            break;
#ifdef QLIB_SUPPORT_OPI
        case PLAT_SPI_FORMAT_8_8_8:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_8_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_8_LINES;
            break;
#endif
        default:
            return -1;
            break;
    }

    // Handle case where there is no CMD
    if (cmdSize == 0)
    {
        sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_NONE;
    }

    // Setup address size
    switch (addressSize)
    {
        case 0:
            sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;
            break;
        case 3:
            sCommand.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
            sCommand.Address            = MAKE32B(dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize], 0);
            break;
        case 4:
            sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
            sCommand.Address            = MAKE32B(dataOutStream[(cmdSize+3)], dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize]);;
            break;
        default:
            return -1;  // not supported
            break;
    }


    // TODO: handle Alt Bytes (i.e: to support SSE where there can be dataOut and dataIn at the same time)
    if (dataInSize > 0)
    {
        sCommand.DataLength = dataInSize;
    }
    else if (dataOutSize > 0)
    {
        sCommand.DataLength = dataOutSize;
    }
    else
    {
        sCommand.DataLength = 0;
        sCommand.DataMode = HAL_XSPI_DATA_NONE; // no data phase
    }

    // Setup dummy cycles
    sCommand.DummyCycles = dummyCycles;

    // Setup DQS enable/disable (pre-defined)
    sCommand.DQSMode = g_dqs_mode;

    // Process the SPI transaction
    // Send the command (1st part of transaction)
    if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return -1;  // error
    }

    // Read/write data (2nd part of transaction)
    if (dataInSize > 0)  // read data
    {
        if (HAL_XSPI_Receive(hxspi, dataIn, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return -1;  // error
        }
    }
    else if (dataOutSize > 0)  // write data
    {
        if (HAL_XSPI_Transmit(hxspi, dataOutStream + cmdSize + addressSize, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return -1;  // error
        }
    }

    return 0;
}
