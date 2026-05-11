/************************************************************************************************************
* @copyright  Copyright (c) 2026 by Winbond Electronics Corporation . All rights reserved
*
* @file       pin_support.h
* @brief      This file contains header for Leds pin support on STM32N6 device
*
************************************************************************************************************/

#ifndef _PIN_SUPPORT_H_
#define _PIN_SUPPORT_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Declarations
 ******************************************************************************/

void STM32_RedLed_TurnOnOff(unsigned char onOff);
void STM32_GreenLed_TurnOnOff(unsigned char onOff);
void STM32_BlueLed_TurnOnOff(unsigned char onOff);

#endif /* _PIN_SUPPORT_H_ */

