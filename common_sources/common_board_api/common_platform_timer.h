/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_timer.h
* @brief      This file includes platform specific definitions
*
* ### project W77Q
*
************************************************************************************************************/
#ifndef COMMON_PLATFORM_TIMER_H__
#define COMMON_PLATFORM_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include <stdint.h>

/************************************************************************************************************
 * timer ID number - number of timers available is depends on the specific platform
 ***********************************************************************************************************/
typedef enum PLAT_TIMER_ID_T
{
    PLAT_TIMER_ID_FIRST     = 0x30424309,

    PLAT_TIMER_ID0,
    PLAT_TIMER_ID1,
    PLAT_TIMER_ID2,
    PLAT_TIMER_ID3,
    PLAT_TIMER_ID4,

    PLAT_TIMER_ID_LAST
} PLAT_TIMER_ID_T;

/************************************************************************************************************
 * Time resolution
 ***********************************************************************************************************/
typedef enum PLAT_TIMER_RES_T
{
    PLAT_TIMER_RES_FIRST      = 0x6e9afa11,

    PLAT_TIMER_RES_SEC,   ///< Resolution in seconds
    PLAT_TIMER_RES_MILLI, ///< Resolution in milliseconds
    PLAT_TIMER_RES_MICRO, ///< Resolution in microseconds
    PLAT_TIMER_RES_NANO,  ///< Resolution in nanoseconds
    PLAT_TIMER_RES_TICKS, ///< Resolution in clock ticks

    PLAT_TIMER_RES_LAST
} PLAT_TIMER_RES_T;

/************************************************************************************************************
 * timer operation mode
 ***********************************************************************************************************/
typedef enum PLAT_TIMER_MODE_T
{
    PLAT_TIMER_MODE_FIRST = 0x60542427,

    PLAT_TIMER_MODE_ONE_SHOT,       ///< Unregister after execution
    PLAT_TIMER_MODE_PERIODIC,       ///< Reset on match
    PLAT_TIMER_MODE_PERIODIC_GROUP, ///< Execute on each match without reset

    PLAT_TIMER_MODE_LAST
} PLAT_TIMER_MODE_T;

/************************************************************************************************************
 * @brief       Interrupt service routine for timer interrupt
 *
 * @return      none
 ***********************************************************************************************************/
typedef void (*PLAT_TIMER_ISR_T)(void);

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       initializes the timer module
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 * @param[in]   timeRes         timer counter resolution. see @ref PLAT_TIMER_RES_T for resolution options
 *
 * @return
 * QLIB_STATUS__OK = 0                  - no error occurred\n
 * QLIB_STATUS__PARAMETER_OUT_OF_RANGE  - @p timerId is not available for this platform\n
 * QLIB_STATUS__INVALID_PARAMETER       - The require resolution (@p timeRes) is not possible for this platform\n
 * QLIB_STATUS__(ERROR)                 - Other error
 ***********************************************************************************************************/
int PLAT_TIMER_Init(PLAT_TIMER_ID_T timerId, PLAT_TIMER_RES_T timeRes);

/************************************************************************************************************
 * @brief       Un-initializes the timer module
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 *
 * @return      0 if no error occurred, QLIB_STATUS__(ERROR) otherwise
 ***********************************************************************************************************/
int PLAT_TIMER_Finit(PLAT_TIMER_ID_T timerId);

/************************************************************************************************************
 * @brief       Starts the timer
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 *
 * @return      0 if no error occurred, QLIB_STATUS__(ERROR) otherwise
 ***********************************************************************************************************/
int PLAT_TIMER_Start(PLAT_TIMER_ID_T timerId);

/************************************************************************************************************
 * @brief       Stops the timer
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 *
 * @return      0 if no error occurred, QLIB_STATUS__(ERROR) otherwise
 ***********************************************************************************************************/
int PLAT_TIMER_Stop(PLAT_TIMER_ID_T timerId);

/************************************************************************************************************
 * @brief       Reads the current timer counter value
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 * @param[out]  timerCounter    The current timer counter value, the units are defined during @ref PLAT_TIMER_Init
 *
 * @return      0 if no error occurred, QLIB_STATUS__(ERROR) otherwise
 ***********************************************************************************************************/
int PLAT_TIMER_Get(PLAT_TIMER_ID_T timerId, uint32_t* timerCounter);


/************************************************************************************************************
 * @brief       register an interrupt event for specific a match value of a timer
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 * @param[in]   matchValue      When the timer counter value get to this value @p callback function will be called
 * @param[in]   mode            If PLAT_TIMER_MODE_PERIODIC then the counter will be set to zero when the @p matchValue is reached
 * @param[in]   callback        Callback function to execute when the @p matchValue is reached
 *
 * @return
 * QLIB_STATUS__OK = 0                  - no error occurred\n
 * QLIB_STATUS__PARAMETER_OUT_OF_RANGE  - If all the match slots are used (try to unregister one interrupt)\n
 * QLIB_STATUS__(ERROR)                 - Other error
 ***********************************************************************************************************/
int PLAT_TIMER_RegisterInterrupt(PLAT_TIMER_ID_T    timerId,
                                 uint32_t           matchValue,
                                 PLAT_TIMER_MODE_T  mode,
                                 PLAT_TIMER_ISR_T   callback);

/************************************************************************************************************
 * @brief       Un-register an interrupt event for specific a match value of a timer
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 * @param[in]   matchValue      Match value of the timer @p timerId to be disabled
 *
 * @return
 * QLIB_STATUS__OK = 0                  - no error occurred\n
 * QLIB_STATUS__INVALID_PARAMETER       - If the match slots is not found\n
 * QLIB_STATUS__(ERROR)                 - Other error
 ***********************************************************************************************************/
int PLAT_TIMER_UnregisterInterrupt(PLAT_TIMER_ID_T timerId, uint32_t matchValue);

/************************************************************************************************************
 * @brief       Block execution for certain amount of time
 *              Note: If there is any register interrupt to the timer @p timerId then the behavior of this timer is unknown
 *
 * @param[in]   timerId         Id of timer, available timers depends on the specific platform
 * @param[in]   delayTime       The delay time, the units of this parameter are defined during @ref PLAT_TIMER_Init
 *
 * @return      0 if no error occurred, QLIB_STATUS__(ERROR) otherwise
 ***********************************************************************************************************/
int PLAT_TIMER_Delay(PLAT_TIMER_ID_T timerId, uint32_t delayTime);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif  // COMMON_PLATFORM_TIMER_H__
