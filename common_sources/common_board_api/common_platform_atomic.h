/************************************************************************************************************
* @internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2026 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       common_platform_atomic.h
* @brief      This file includes platform specific atomic function definitions
*
* ### project common_board_api
*
************************************************************************************************************/
#ifndef __COMMON_PLATFORM_ATOMIC_H__
#define __COMMON_PLATFORM_ATOMIC_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include <stdbool.h>

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief Atomic compare and exchange operation
 *
 * This macro performs an atomic compare-and-exchange operation on a boolean variable.
 * If the current value equals the expected value, it updates to the desired value and returns true.
 * Otherwise, it returns false without modifying the variable.
 *
 * The implementation should be atomic and thread/interrupt safe.
 * The default implementation is not atomic - suitable when QLIB is used as single user of the SPI bus.
 * User should override this macro with proper atomic operation if needed.
 *
 * @param[in,out] ptr       Pointer to a bool variable to operate on
 * @param[in]     expected  Expected current value
 * @param[in]     desired   Desired new value
 *
 * @return true if exchange was successful (value was expected and updated to desired), false otherwise
 *
 ************************************************************************************************************/
bool PLAT_AtomicCompareExchange(bool* ptr, bool expected, bool desired);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif // __COMMON_PLATFORM_ATOMIC_H__
