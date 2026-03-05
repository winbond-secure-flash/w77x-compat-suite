#include "common_platform_led.h"
#include "stm32h7s78_discovery.h"
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

void STM32_BlueLed_TurnOnOff(unsigned char onOff)
{
    if (onOff)
        BSP_LED_On(LED_BLUE);
    else
        BSP_LED_Off(LED_BLUE);
}

int PLAT_LED_turn_on_off_blue_led(bool on_off)
{
    STM32_BlueLed_TurnOnOff(on_off);
    return 0;
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
