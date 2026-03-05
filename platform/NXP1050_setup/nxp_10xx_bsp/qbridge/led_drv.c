/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2021 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       led_drv.c
* @brief      This file includes platform specific features implementations for LED on NXP 10xx device
*
* ### project W77Q
*
************************************************************************************************************/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"
#include "common_platform_led.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
int LED_turn_on_off_L(bool on_off, int pin);
int LED_is_turned_on_off_L(bool* on_off, int pin);

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                              DEFINITIONS                                                */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define USER_LED_GPIO_PORT           GPIO2
#define USER_LED_BLUE_GPIO_PIN      (20U)
#define USER_LED_GREEN_GPIO_PIN     (22U)
#define USER_LED_RED_GPIO_PIN       (23U)

void PLAT_LED_init(void)
{
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_04_GPIO2_IO20, 0U); // led_blue
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_06_GPIO2_IO22, 0U); // led_green
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_07_GPIO2_IO23, 0U); // led_red

    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_04_GPIO2_IO20, 0xB0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_06_GPIO2_IO22, 0xB0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_07_GPIO2_IO23, 0xB0U);

    GPIO_PinInit(USER_LED_GPIO_PORT, USER_LED_BLUE_GPIO_PIN, &led_config);
    GPIO_PinInit(USER_LED_GPIO_PORT, USER_LED_GREEN_GPIO_PIN, &led_config);
    GPIO_PinInit(USER_LED_GPIO_PORT, USER_LED_RED_GPIO_PIN, &led_config);
}

int PLAT_LED_turn_on_off_blue_led(bool on_off)
{
    return LED_turn_on_off_L(on_off, USER_LED_BLUE_GPIO_PIN);
}

int PLAT_LED_turn_on_off_red_led(bool on_off)
{
    return LED_turn_on_off_L(on_off, USER_LED_RED_GPIO_PIN);
}

int PLAT_LED_turn_on_off_green_led(bool on_off)
{
    return LED_turn_on_off_L(on_off, USER_LED_GREEN_GPIO_PIN);
}

int PLAT_LED_is_turned_on_blue_led(bool* pTurned_on)
{
    return LED_is_turned_on_off_L(pTurned_on, USER_LED_BLUE_GPIO_PIN);
}

int PLAT_LED_is_turned_on_red_led(bool* pTurned_on)
{
    return LED_is_turned_on_off_L(pTurned_on, USER_LED_RED_GPIO_PIN);
}

int PLAT_LED_is_turned_on_green_led(bool* pTurned_on)
{
    return LED_is_turned_on_off_L(pTurned_on, USER_LED_GREEN_GPIO_PIN);
}

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                     LOCAL FUNCTIONS IMPLEMENTATION                                      */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
int LED_turn_on_off_L(bool on_off, int pin)
{
    if(true == on_off)
    {//Turn on
        GPIO_PinWrite(USER_LED_GPIO_PORT, pin, 1U);
    }
    else
    {
        GPIO_PinWrite(USER_LED_GPIO_PORT, pin, 0U);
    }

    return 0;
}

int LED_is_turned_on_off_L(bool* on_off, int pin)
{
    if (1 == GPIO_PinRead(USER_LED_GPIO_PORT, pin))
    {
        *on_off = true;
    }
    else
    {
        *on_off = false;
    }

    return 0;
}

//Uncomment to add LED_test code.
/*
int LED_test(void)
{
  int status = 0;

  PLAT_LED_init();
  status += PLAT_LED_turn_on_off_blue_led(1);
  for(int i = 0; i < 1000; ++i);
  status += PLAT_LED_turn_on_off_blue_led(0);

  status += PLAT_LED_turn_on_off_red_led(1);
  for(int i = 0; i < 1000; ++i);
  status += PLAT_LED_turn_on_off_red_led(0);

  status += PLAT_LED_turn_on_off_green_led(1);
  for(int i = 0; i < 1000; ++i);
  status += PLAT_LED_turn_on_off_green_led(0);

  return status;
}
*/
