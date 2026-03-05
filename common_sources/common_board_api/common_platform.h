/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2023 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform.h
* @brief      This file includes platform specific definitions
************************************************************************************************************/
#ifndef COMMON_PLATFORM_H__
#define COMMON_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <Windows.h>
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 DEFINES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

#ifndef ENUM_IN_RANGE
#define ENUM_IN_RANGE(e, prefix) ((((e) > (prefix##_FIRST)) && ((e) < (prefix##_LAST))))
#endif // ENUM_IN_RANGE

#ifndef ASSERT_RET
#define ASSERT_RET(cond, err)      \
    {                              \
        if (!(cond))               \
        {                          \
            return err;            \
        }                          \
    }
#endif // ASSERT_RET

#ifndef STATUS_RET_CHECK
#define STATUS_RET_CHECK(func)                  \
    {                                           \
        int qlib_error_check_ret;                             \
        if (0 != (qlib_error_check_ret = func))               \
        {                                       \
            return qlib_error_check_ret;                      \
        }                                       \
        (void)qlib_error_check_ret;                   \
    }
#endif // STATUS_RET_CHECK

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif // COMMON_PLATFORM_H__
