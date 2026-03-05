/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2021 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_utils.h
* @brief      This file includes platform specific definitions
*
* ### project W77Q
*
************************************************************************************************************/
#ifndef COMMON_PLATFORM_UTILS_H__
#define COMMON_PLATFORM_UTILS_H__

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
/*                                               DEFINITIONS                                               */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

typedef enum
{
    HW_SOCKET_TYPE_TFBGA         = 0x00,
    HW_SOCKET_TYPE_SOP16         = 0x10,
    HW_SOCKET_TYPE_SOP8          = 0x11,
    HW_SOCKET_TYPE_WSON8         = 0x20,
    HW_SOCKET_TYPE_WSON6         = 0x21,
    HW_SOCKET_TYPE_UNKNOWN       = 0xFF
}HW_SOCKET_TYPE_T;


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       Initialize platform
 *
 * @param[out]  user_data        User data which should be saved using QLIB_SetUserData
 * @param[in]   spi_freq         SPI frequency in hz
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_Init(void* user_data, uint32_t spi_freq_hz);

/************************************************************************************************************
 * @brief       Finalize platform
 *
 * @param[out]  user_data        User data
 *
 * @return      None
************************************************************************************************************/
void PLAT_Finit(void* user_data);

/************************************************************************************************************
 * @brief       Wait until switch on the board is pressed
 *
 * @return      None
************************************************************************************************************/
void PLAT_waitForSwitch(void);

/************************************************************************************************************
 * @brief       retrieves flash type, it can be secured or standard flash
 *
 * @param[out]  val        value of flash type (true for secure flash, false for standard flash)
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_FlashType(bool* val);

/************************************************************************************************************
 * @brief       returns CPU frequency
 *
 * @return      CPU frequency
************************************************************************************************************/
uint32_t PLAT_GetCpuFreq(void);

/************************************************************************************************************
 * @brief Block execution for @p mSec milliseconds
 *
 * @param[in]   mSec         Id of timer, available timers depends on the specific platform
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_DelayMsec(uint32_t mSec);

/************************************************************************************************************
 * @brief Hash with SHA256
 *
 * @param[out]  output       Hash data
 * @param[in]   data         data buffer
 * @param[in]   dataSize     data buffer size
 * @param[in]   padValue     4 bytes padding after data value
 * @param[in]   padSize      padding after data size
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_SHA256WithPadding(uint32_t* output, const void* data, uint32_t dataSize, uint32_t padValue, uint32_t padSize);

/************************************************************************************************************
 * @brief True Random Number Generator (TRNG) Get Random Data
 *
 * @param[out]  output          Random data buffer
 * @param[in]   bufferSize      buffer size
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_TRNG_GetRandomData(uint8_t* output, uint32_t bufferSize);

/************************************************************************************************************
 * @brief Get Hardware Socket Type
 *
 * @param[out]  socket type     TFBGA/SOP16/SOP8/WSON8/WSON6/Unknown
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GetHwSocketType(HW_SOCKET_TYPE_T* hwSocketType);

/************************************************************************************************************
 * @brief Toggle Flash Power
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_ToggleFlashPower(void);

/************************************************************************************************************
 * @brief Set SPI IOs as GPIOs except MISO and MOSI IOs or set back to SPI
 *
 * @param[in]   setAsGPIO      True - set IOs as GPIOs, False - set IOs back to SPI
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_SetSpiIOsAsGPIOsExceptMosiAndMisoOrBackToSPI(bool setAsGPIO);

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                         QLIB DEFINE OVERRIDES                                           */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/************************************************************************************************************
 * @brief   This macro holds execution for @p msec milliseconds.
 *          This is required for WD calibration.
 * @param   mSec   time to hold
************************************************************************************************************/
#ifdef QLIB_SAMPLE_DELAY_MSEC
#undef QLIB_SAMPLE_DELAY_MSEC
#endif
#define QLIB_SAMPLE_DELAY_MSEC(mSec)            PLAT_DelayMsec(mSec)

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif // COMMON_PLATFORM_UTILS_H__
