/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2025 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_func.h
* @brief      This file includes definitions for commands implementations
*
*
************************************************************************************************************/
#ifndef __COMP_TEST_FUNC_H__
#define __COMP_TEST_FUNC_H__
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "comp_test_main.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 DEFINES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/* HW Version Register (HW_VER) fields                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
#define QLIB_REG_HW_VER__REVISION   0u, 4u
#define QLIB_REG_HW_VER__HASH_VER   4u, 4u
#define QLIB_REG_HW_VER__SEC_VER    8u, 8u
#define QLIB_REG_HW_VER__FLASH_SIZE 16u, 4u
#define QLIB_REG_HW_VER__FLASH_VER  20u, 4u
#define QLIB_REG_HW_VER__RESERVED_1 24u, 7u
#define QLIB_REG_HW_VER__DB         31u, 1u
/*---------------------------------------------------------------------------------------------------------*/
/* ACTR Register (ACTR) fields                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
#define REG_ACTR_GS_OCTAL   11u, 1u

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
typedef uint32_t HW_VER_T; ///< 32b
typedef uint32_t ACTR_T; ///< 32b
/************************************************************************************************************
 * Hardware Version - Standard part
************************************************************************************************************/
typedef struct
{
    uint8_t manufacturerID; ///< Manufacture ID
    uint8_t memoryType;     ///< Memory type
    uint8_t capacity;       ///< Capacity
    uint8_t deviceID;       ///< Device ID
} COMP_TEST_STD_HW_VER_T;

/************************************************************************************************************
 * Hardware Version - Secure part
************************************************************************************************************/
typedef struct
{
    uint8_t flashVersion;    ///< Flash version
    uint8_t securityVersion; ///< Security Protocol Version
    uint8_t revision;        ///< Hardware revision
    uint8_t flashSize;       ///< Flash size
    uint8_t hashVersion;     ///< Hash function version
} COMP_TEST_SEC_HW_VER_T;

/************************************************************************************************************
 * Flash Size
************************************************************************************************************/
typedef enum
{
    COMP_TEST_FLASH_SIZE_UNKNOWN = 0,
    COMP_TEST_FLASH_SIZE_32Mb    = 0x15,
    COMP_TEST_FLASH_SIZE_64Mb,
    COMP_TEST_FLASH_SIZE_128Mb,
    COMP_TEST_FLASH_SIZE_256Mb,
    COMP_TEST_FLASH_SIZE_512Mb,
    COMP_TEST_FLASH_SIZE_1Gb
} COMP_TEST_FLASH_SIZE_T;

#define COMP_DEVICE_ID_TO_FLASH_SIZE(devId) ((COMP_TEST_FLASH_SIZE_T)(devId))
/************************************************************************************************************
 * Flash Revision
************************************************************************************************************/
typedef enum
{
    COMP_TEST_FLASH_REVISION_UNKNOWN = 0xFF,
    COMP_TEST_FLASH_REVISION_A       = 1,
    COMP_TEST_FLASH_REVISION_B,
    COMP_TEST_FLASH_REVISION_C,
    COMP_TEST_2_FLASH_REVISION_A  = COMP_TEST_FLASH_REVISION_A,
    COMP_TEST_2_FLASH_REVISION_B  = COMP_TEST_FLASH_REVISION_B,
    COMP_TEST_2_FLASH_REVISION_C  = COMP_TEST_FLASH_REVISION_C,
    COMP_TEST_3_FLASH_REVISION_A  = COMP_TEST_FLASH_REVISION_A,
    COMP_TEST_3_FLASH_REVISION_B  = COMP_TEST_FLASH_REVISION_B,
    COMP_TEST_3_FLASH_REVISION_D  = COMP_TEST_FLASH_REVISION_C,
    COMP_TEST_31_FLASH_REVISION_A = COMP_TEST_FLASH_REVISION_A
} COMP_TEST_FLASH_REVISION_T;

/************************************************************************************************************
 * Flash Supply Voltage
************************************************************************************************************/
typedef enum
{
    COMP_TEST_FLASH_VOLTAGE_UNKNOWN,
    COMP_TEST_FLASH_VOLTAGE_1_8V,
    COMP_TEST_FLASH_VOLTAGE_3_3V
} COMP_TEST_FLASH_VOLTAGE_T;

/************************************************************************************************************
 * Hardware Version - General Information
************************************************************************************************************/
typedef struct
{
    COMP_TEST_FLASH_SIZE_T     flashSize;   ///< Flash size
    bool                       isSingleDie; ///< Single/Multi die
    COMP_TEST_FLASH_SIZE_T     dieSize;     ///< Single Die size
    COMP_TEST_FLASH_VOLTAGE_T  voltage;     ///< Flash voltage
    COMP_TEST_FLASH_REVISION_T revision;    ///< Target revision
    uint32_t                   dieType;     ///< DIE type as defined in @ref targets.h
} COMP_TEST_HW_INFO_T;

/************************************************************************************************************
 * Hardware Version
************************************************************************************************************/
typedef struct
{
    COMP_TEST_STD_HW_VER_T std;  ///< standard HW information
    COMP_TEST_SEC_HW_VER_T sec;  ///< secure HW version
    COMP_TEST_HW_INFO_T    info; ///< HW information
} COMP_TEST_HW_VER_T;


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       FUNCTIONS DECLARATIONS                                            */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_STATUS_T GetHwVersionRegister(uint32_t* HwVersion);
COMP_TEST_STATUS_T GetVersion(COMP_TEST_HW_VER_T* pCompTestHwVer);
COMP_TEST_STATUS_T GetActrRegister(ACTR_T* pActrReg);

COMP_TEST_STATUS_T COMP_TEST_SetDummy(QLIB_BUS_MODE_T busMode, uint8_t dummy);
COMP_TEST_STATUS_T COMP_TEST_SetDqsEnable(QLIB_BUS_MODE_T busMode, bool dqsEn);
COMP_TEST_STATUS_T COMP_TEST_GetDqsEnable(QLIB_BUS_MODE_T busMode, bool *dqsEn);
COMP_TEST_STATUS_T COMP_TEST_SetQeEnable(QLIB_BUS_MODE_T busMode);
COMP_TEST_STATUS_T COMP_TEST_GetQeEnable(QLIB_BUS_MODE_T busMode, uint8_t *qeFlag);



#endif // __COMP_TEST_FUNC_H__
