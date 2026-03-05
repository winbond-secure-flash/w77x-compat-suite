/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_gpio.h
* @brief      This file includes platform specific definitions
*
* ### project W77Q
*
************************************************************************************************************/
#ifndef COMMON_PLATFORM_GPIO_H__
#define COMMON_PLATFORM_GPIO_H__

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
/* Build GPIO number from port/bit pair                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define PLAT_GPIO_NUM(port,  bit)            ((port) << 8 | (bit))
#define PLAT_GPIO_PORT(gpNum)                ((gpNum) >> 8)
#define PLAT_GPIO_BIT(gpNum)                 ((gpNum) & 0xFF)

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

#if 0
// TODO - add GPIO interrupt support
/************************************************************************************************************
 * @brief       Interrupt service routine for GPIO interrupt
 *
 * @return      none
 ************************************************************************************************************/
typedef void (*PLAT_GPIO_ISR_T)(void);
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       initializes the GPIO module
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_Init(void* user_data);

/************************************************************************************************************
 * @brief       Un-initializes the GPIO module
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_Finit(void);

/************************************************************************************************************
 * @brief       configures a single GPIO pin as input
 *
 * @param[in]   gpioPinNum      Pin identifier, see @ref PLAT_GPIO_NUM
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_ConfigInput(uint32_t gpioPinNum);

/************************************************************************************************************
 * @brief       configures a single GPIO pin as output
 *
 * @param[in]   gpioPinNum      Pin identifier, see @ref PLAT_GPIO_NUM
 * @param[in]   initVal         Pin initial output value (true for High, false for Low)
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_ConfigOutput(uint32_t gpioPinNum, bool initVal);

/************************************************************************************************************
 * @brief       Reads a single GPIO pin state
 *
 * @param[in]   gpioPinNum      Pin identifier, see @ref PLAT_GPIO_NUM
 * @param[out]  val             Current value of the GPIO pin (true for High, false for Low)
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_Read(uint32_t gpioPinNum, bool* val);

/************************************************************************************************************
 * @brief       Changes a single GPIO pin state
 *
 * @param[in]   gpioPinNum      Pin identifier, see @ref PLAT_GPIO_NUM
 * @param[in]   pinType         New pin value (true for High, false for Low)
 *
 * @return      0 if no error occurred
 ************************************************************************************************************/
int PLAT_GPIO_Write(uint32_t gpioPinNum, bool val);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif  // COMMON_PLATFORM_GPIO_H__
