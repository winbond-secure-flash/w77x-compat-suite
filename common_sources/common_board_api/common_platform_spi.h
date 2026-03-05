/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2024 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_spi.h
* @brief      This file includes SPI platform specific definitions
*
* ### project W77Q
*
************************************************************************************************************/
#ifndef COMMON_PLATFORM_SPI_H__
#define COMMON_PLATFORM_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "common_platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       DEFINITIONS                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/************************************************************************************************************
 * Fetch command SPI format
 ************************************************************************************************************/
typedef enum PLAT_SPI_FORMAT_T
{
    PLAT_SPI_FORMAT_INVALID = 0,
    PLAT_SPI_FORMAT_1_1_1   = 1,
    PLAT_SPI_FORMAT_1_1_2   = 2,
    PLAT_SPI_FORMAT_1_2_2   = 3,
    PLAT_SPI_FORMAT_1_1_4   = 4,
    PLAT_SPI_FORMAT_1_4_4   = 5,
    PLAT_SPI_FORMAT_4_4_4   = 6,
    PLAT_SPI_FORMAT_1_8_8   = 7,
    PLAT_SPI_FORMAT_8_8_8   = 8,
    PLAT_SPI_FORMAT_MAX     = PLAT_SPI_FORMAT_8_8_8
} PLAT_SPI_FORMAT_T;


/************************************************************************************************************
 * This type contains dual transfer rate for SPI commands
************************************************************************************************************/
typedef enum PLAT_SPI_DTR_T
{
    PLAT_SPI_DTR__NO_DTR,    // CMD, ADDR and DATA in STR
    PLAT_SPI_DTR__ADDR_DATA, // CMD in STR, ADDR and DATA in DTR
    PLAT_SPI_DTR__ALL        // CMD, ADDR and DATA in DTR
} PLAT_SPI_DTR_T;

/************************************************************************************************************
 * SPI interface parameters
************************************************************************************************************/
typedef struct PLAT_SPI_T
{
    PLAT_SPI_FORMAT_T mode;
    PLAT_SPI_DTR_T    dtr;
    uint32_t          dummyCycles;
    bool              addrMode3Bytes;
} PLAT_SPI_T;

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       Set Core Fetch Command for direct flash access and XIP execution
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_SPI_SetFetchCmd(const PLAT_SPI_T* spiIf);

int PLAT_SPI_DirectWriteReadTransaction(const void*     userData,
                                  PLAT_SPI_FORMAT_T format,
                                  uint32_t        flags,
                                  const uint8_t*  dataOutStream,
                                  uint32_t        cmdSize,
                                  uint32_t        addressSize,
                                  uint32_t        dataOutSize,
                                  uint32_t        dummyCycles,
                                  uint8_t*        dataIn,
                                  uint32_t        dataInSize);

int PLAT_SPI_EnableDQS(bool enable);
int PLAT_SPI_SetFreqInHz(uint32_t freq);
int PLAT_SPI_GetFreqInHz(uint32_t* freq);
bool PLAT_SPI_GetDqsMode(void);
int PLAT_SPI_SetChipSelect(uint8_t chipSelect);

typedef struct PLAT_SPI_BUSMODE_SUPPORTED_T
{
    uint32_t plat_spi_1_1_1 : 1; //1
    uint32_t plat_spi_1_1_2 : 1; //2
    uint32_t plat_spi_1_2_2 : 1; //3
    uint32_t plat_spi_1_1_4 : 1; //4
    uint32_t plat_spi_1_4_4 : 1; //5
    uint32_t plat_spi_4_4_4 : 1; //6
    uint32_t plat_spi_1_8_8 : 1; //7
    uint32_t plat_spi_8_8_8 : 1; //8
    uint32_t reserved : 24;
} PLAT_SPI_BUSMODE_SUPPORTED_T;

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif  // COMMON_PLATFORM_SPI_H__
