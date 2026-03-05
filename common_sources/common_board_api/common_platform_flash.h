/************************************************************************************************************
* @internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2025 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       common_platform_flash.h
* @brief      This file contains platform specific definitions for flash access
*
************************************************************************************************************/

#ifndef COMMON_PLATFORM_FLASH_H__
#define COMMON_PLATFORM_FLASH_H__

/************************************************************************************************************
*************************************************************************************************************
 *                                                INCLUDES
*************************************************************************************************************
************************************************************************************************************/
#include "common_platform.h"

/************************************************************************************************************
*************************************************************************************************************
 *                                       PLATFORM SPECIFIC FUNCTIONS
*************************************************************************************************************
************************************************************************************************************/

/************************************************************************************************************
 * @brief       This routine reads data from Flash.
 *
 * @param[out]      data            Output data
 * @param[in]       addr            Address to read from
 * @param[in]       size            Data size in bytes
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Read(uint8_t* data, const uint32_t addr, const uint32_t size);

/************************************************************************************************************
 * @brief       This routine writes data to Flash
 *
 * @param[in]       data            Data to write
 * @param[in]       addr            Address to write to
 * @param[in]       size            Data size in bytes
 * @param[in]       auth            If TRUE, authenticated write is performed
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Write(const uint8_t* data, const uint32_t addr, const uint32_t size);

/************************************************************************************************************
 * @brief       This routine erases area of the Flash
 *
 * @param[in]       addr            Address to erase
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Erase(const uint32_t addr);

/************************************************************************************************************
 * @brief       This routine suspends on-going Flash operation
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Suspend(void);

/************************************************************************************************************
 * @brief       This routine resumes suspended Flash operation
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Resume(void);

/************************************************************************************************************
 * @brief       This routine waits while Flash operation is in progress
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Wait(void);

/************************************************************************************************************
 * @brief       This routine perform flash recovery after suspend or power down. If parameter
 *              forceRecovery is set on FALSE, flash in NOT suspended state will not recovered. This
 *              is done to save time
 *
 * @param[in]       addr            Address of the sector to undergo recovery
 * @param[in]       forceRecovery   true,  Hw Flash recovery will be executed always. Used on powerup
 *                                  false, Hw flash recovery will not be executed if flash is not
 *                                         suspended. Used after suspension to avoid additional and not
 *                                         needed Hw flash recovery. This can happen when HW finishes
 *                                         erasure before software updates its status
 *
 * @return      0 if no error occurred
************************************************************************************************************/
int PLAT_FLASH_Recovery(uint32_t addr, bool force_recovery);

#endif // COMMON_PLATFORM_FLASH_H__
