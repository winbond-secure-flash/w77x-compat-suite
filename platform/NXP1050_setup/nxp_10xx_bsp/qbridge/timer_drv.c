/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       timer_dvr.c
* @brief      This file includes platform specific implementations for timer on NXP 10xx device
*
* ### project W77Q
*
************************************************************************************************************/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "common_platform_timer.h"
#include "fsl_gpt.h"
#include "MIMXRT1052.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 DEFINES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define GET_TIME_RES(timeResId)                \
(                                              \
    (PLAT_TIMER_RES_MILLI == timeResId) ? 66 : \
   	(PLAT_TIMER_RES_MICRO == timeResId) ? 75 : \
   	0                                          \
)

#define GET_TIMER_GPT(timerId)                 \
(                                              \
    (PLAT_TIMER_ID0 == timerId) ? GPT1 :       \
        (PLAT_TIMER_ID1 == timerId) ? GPT2 :   \
            NULL                               \
)

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

int PLAT_TIMER_Init(PLAT_TIMER_ID_T timerId, PLAT_TIMER_RES_T timeRes)
{
    gpt_config_t gptConfig;
    GPT_Type *timer_gpt = GET_TIMER_GPT(timerId);

    GPT_GetDefaultConfig(&gptConfig);

    /* Initialize GPT module */
    GPT_Init(timer_gpt, &gptConfig);

    /* Divide GPT clock source frequency inside GPT module to measure correct time resolution */
    GPT_SetClockDivider(timer_gpt, GET_TIME_RES(timeRes));

    return 0;
}

int PLAT_TIMER_Finit(PLAT_TIMER_ID_T timerId)
{
    return 0;
}

int PLAT_TIMER_Start(PLAT_TIMER_ID_T timerId)
{
    GPT_Type *timer_gpt = GET_TIMER_GPT(timerId);
    GPT_StartTimer(timer_gpt);

    return 0;
}

int PLAT_TIMER_Stop(PLAT_TIMER_ID_T timerId)
{
    GPT_Type *timer_gpt = GET_TIMER_GPT(timerId);
    GPT_StopTimer(timer_gpt);

    return 0;
}

int PLAT_TIMER_Get(PLAT_TIMER_ID_T timerId, uint32_t* timerCounter)
{
    GPT_Type *timer_gpt = GET_TIMER_GPT(timerId);
    *timerCounter = GPT_GetCurrentTimerCount(timer_gpt);

    return 0;
}

int PLAT_TIMER_RegisterInterrupt(PLAT_TIMER_ID_T    timerId,
                                 uint32_t           matchValue,
                                 PLAT_TIMER_MODE_T  mode,
                                 PLAT_TIMER_ISR_T   callback)
{
    // TBD
    return 0;
}

int PLAT_TIMER_UnregisterInterrupt(PLAT_TIMER_ID_T timerId, uint32_t matchValue)
{
    // TBD
    return 0;
}

int PLAT_TIMER_Delay(PLAT_TIMER_ID_T timerId, uint32_t delayTime)
{
    // TBD
    return 0;
}
