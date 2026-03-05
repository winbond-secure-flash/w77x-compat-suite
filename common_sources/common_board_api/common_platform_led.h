/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_led.h
* @brief      This file includes platform specific features definitions
*
* ### project W77Q
*
************************************************************************************************************/

#ifndef COMMON_PLATFORM_LED_H__
#define COMMON_PLATFORM_LED_H__

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
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
int PLAT_LED_turn_on_off_blue_led(bool on_off);
int PLAT_LED_turn_on_off_red_led(bool on_off);
int PLAT_LED_turn_on_off_green_led(bool on_off);
int PLAT_LED_is_turned_on_blue_led(bool* pTurned_on);
int PLAT_LED_is_turned_on_red_led(bool* pTurned_on);
int PLAT_LED_is_turned_on_green_led(bool* pTurned_on);

void PLAT_LED_init(void);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif  // COMMON_PLATFORM_LED_H__
