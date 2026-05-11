/************************************************************************************************************
* @copyright  Copyright (c) 2026 by Winbond Electronics Corporation . All rights reserved
*
* @file       pin_support.c
* @brief      This file contains implementation for Leds pin support on STM32N6 device
*
************************************************************************************************************/

#include "common_platform_led.h"
#include "stm32n6570_discovery.h"
#include "pin_support.h"

/*******************************************************************************
 * Implementations
 ******************************************************************************/

void STM32_RedLed_TurnOnOff(unsigned char onOff)
{
    if (onOff)
        BSP_LED_On(LED_RED);
    else
        BSP_LED_Off(LED_RED);
}

void STM32_GreenLed_TurnOnOff(unsigned char onOff)
{
    if (onOff)
        BSP_LED_On(LED_GREEN);
    else
        BSP_LED_Off(LED_GREEN);
}

int PLAT_LED_turn_on_off_red_led(bool on_off)
{
    STM32_RedLed_TurnOnOff(on_off);
    return 0;
}

int PLAT_LED_turn_on_off_green_led(bool on_off)
{
    STM32_GreenLed_TurnOnOff(on_off);
    return 0;
}
