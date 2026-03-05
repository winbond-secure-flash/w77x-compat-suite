/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_func.c
* @brief      This file uses flash commands to implement functions on flash
*
************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "defs.h"
#include "qlib_platform.h"
#include "comp_test_targets.h"

#include "comp_test_main.h"
#include "comp_test_cmd.h"
#include "comp_test_opcodes.h"
#include "comp_test_func.h"
#include "string.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                               GLOBALS VSRIABLES                                         */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
extern COMP_TEST_GLOBALS_T g_tCompTestGlobals;
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                               DEFINITIONS                                               */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 MACROS                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                CONSTANTS                                                */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       LOCAL FUNCTION DECLARATION                                        */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_STATUS_T GetTargetFlash_L(COMP_TEST_HW_VER_T* pCompTestHwVer);
static void PrintTargetDetected_L(uint32_t target);


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       FUNCTIONS IMPLEMENTATION                                          */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       This function reads HW version register
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T GetHwVersionRegister(HW_VER_T* HwVersion)
{

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WriteBufOpcode(QLIB_BUS_MODE_1_1_1,0xF0000000, NULL, 0));
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ReadBufOpcode(QLIB_BUS_MODE_1_1_1,(uint8_t*)HwVersion,4));

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function reads ACTR register
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T GetActrRegister(ACTR_T* pActrReg)
{

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WriteBufOpcode(QLIB_BUS_MODE_1_1_1,0x3F000000, NULL, 0));
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ReadBufOpcode(QLIB_BUS_MODE_1_1_1,(uint8_t*)pActrReg,4));

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function gets target version by reading JEDEC ID and HW version register
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T GetVersion(COMP_TEST_HW_VER_T* pCompTestHwVer)
{
    uint8_t jedecIdBuf[3];
    HW_VER_T HwVersionReg;
    COMP_TEST_STD_HW_VER_T* stdHwVersion = &(pCompTestHwVer->std);
    COMP_TEST_SEC_HW_VER_T* secHwVer     = &(pCompTestHwVer->sec);

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_GetJedecId(jedecIdBuf));
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_GetDeviceId(&(stdHwVersion->deviceID)));
    stdHwVersion->manufacturerID = jedecIdBuf[0];
    stdHwVersion->memoryType     = jedecIdBuf[1];
    stdHwVersion->capacity       = jedecIdBuf[2];
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "deviceID = 0x%2X",stdHwVersion->deviceID);
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "JEDEC manufacturerID = 0x%2X, memoryType = 0x%2X, capacity = 0x%2X (%d KB)",jedecIdBuf[0],jedecIdBuf[1],jedecIdBuf[2],(1 << jedecIdBuf[2]) >> 10);

    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Read HW_VER using OP1 and OP2 commands");
    COMP_TEST_STATUS_RET_CHECK(GetHwVersionRegister(&HwVersionReg));
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "HW_VER = 0x%08X", HwVersionReg);

    secHwVer->flashVersion    = (uint8_t)READ_VAR_FIELD(HwVersionReg, QLIB_REG_HW_VER__FLASH_VER);
    secHwVer->securityVersion = (uint8_t)READ_VAR_FIELD(HwVersionReg, QLIB_REG_HW_VER__SEC_VER);
    secHwVer->revision        = (uint8_t)READ_VAR_FIELD(HwVersionReg, QLIB_REG_HW_VER__REVISION);
    secHwVer->hashVersion     = (uint8_t)READ_VAR_FIELD(HwVersionReg, QLIB_REG_HW_VER__HASH_VER);
    secHwVer->flashSize = (uint8_t)READ_VAR_FIELD(HwVersionReg, QLIB_REG_HW_VER__FLASH_SIZE);

    // flash size in MB
    //uint32_t flashSizeMB_calc_from_HW_VER = (1 << secHwVer->flashSize) / 8;

    char tmpStr[256];
    if (secHwVer->securityVersion == 0x30)
    {
        sprintf(tmpStr, "(W77Q/T)");
    }
    else if (secHwVer->securityVersion == 0x34)
    {
        sprintf(tmpStr, "(W77Q/T with automotive safety features)");
    }
    else if (secHwVer->securityVersion == 0x35)
    {
        sprintf(tmpStr, "(W77Q/T (Q3.1) with enhanced LMS functionality)");
    }
    else
    {
        sprintf(tmpStr, "(Unknown chip, security version = 0x%0X)", secHwVer->securityVersion);
    }


    pCompTestHwVer->info.flashSize   = COMP_DEVICE_ID_TO_FLASH_SIZE(stdHwVersion->deviceID); // in terms of codes 0x10, 0x11 .. etc (not bytes)
    pCompTestHwVer->info.dieSize     = (COMP_TEST_FLASH_SIZE_T)((uint32_t)pCompTestHwVer->sec.flashSize + 0x10u); // in terms of codes 0x10, 0x11 .. etc (not bytes)
    pCompTestHwVer->info.voltage      = (stdHwVersion->memoryType == 0x4Au) || (stdHwVersion->memoryType == 0x4Bu) ||
                                          (stdHwVersion->memoryType == 0x4Cu) || (stdHwVersion->memoryType == 0x4Du)
                                       ? COMP_TEST_FLASH_VOLTAGE_3_3V
                                       : COMP_TEST_FLASH_VOLTAGE_1_8V;
    switch (secHwVer->revision)
    {
        case 0:
            pCompTestHwVer->info.revision = COMP_TEST_FLASH_REVISION_A;
            break;
        case 1:
            pCompTestHwVer->info.revision = COMP_TEST_FLASH_REVISION_B;
            break;
        case 2:
            pCompTestHwVer->info.revision = COMP_TEST_FLASH_REVISION_C;
            break;
        default:
            pCompTestHwVer->info.revision = COMP_TEST_FLASH_REVISION_UNKNOWN;
            break;
    }
    GetTargetFlash_L(pCompTestHwVer);
    PrintTargetDetected_L(pCompTestHwVer->info.dieType);
    return COMP_TEST_STATUS_OK;
}

COMP_TEST_STATUS_T GetTargetFlash_L(COMP_TEST_HW_VER_T* pCompTestHwVer)
{
    uint32_t      hwVerAsUint = MAKE32B(pCompTestHwVer->sec.securityVersion, pCompTestHwVer->info.dieSize, pCompTestHwVer->info.voltage, 0u);
    COMP_TEST_STATUS_T status = COMP_TEST_STATUS_OK;
    uint32_t* target;

    target = (uint32_t*)&(pCompTestHwVer->info.dieType);

    switch (hwVerAsUint)
    {
        case MAKE32B(0x20u, COMP_TEST_FLASH_SIZE_32Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            if (pCompTestHwVer->info.revision >= COMP_TEST_2_FLASH_REVISION_B)
            {
                *target = w77q32jw_revB;
            }
            else
            {
                status = COMP_TEST_STATUS__NOT_SUPPORTED;
            }
            break;
        case MAKE32B(0x24u, COMP_TEST_FLASH_SIZE_64Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
        case MAKE32B(0x28u, COMP_TEST_FLASH_SIZE_64Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            *target = w77q64jw_revA;
            break;
        case MAKE32B(0x24u, COMP_TEST_FLASH_SIZE_128Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
        case MAKE32B(0x28u, COMP_TEST_FLASH_SIZE_128Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            *target = w77q128jw_revA;
            break;
        case MAKE32B(0x24u, COMP_TEST_FLASH_SIZE_64Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_3_3V, 0u):
        case MAKE32B(0x28u, COMP_TEST_FLASH_SIZE_64Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_3_3V, 0u):
            *target = w77q64jv_revA;
            break;
        case MAKE32B(0x24u, COMP_TEST_FLASH_SIZE_128Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_3_3V, 0u):
        case MAKE32B(0x28u, COMP_TEST_FLASH_SIZE_128Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_3_3V, 0u):
            *target = w77q128jv_revA;
            break;
        case MAKE32B(0x30u, COMP_TEST_FLASH_SIZE_256Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
        case MAKE32B(0x34u, COMP_TEST_FLASH_SIZE_256Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            if (pCompTestHwVer->info.revision == COMP_TEST_3_FLASH_REVISION_B)
            {
               *target = (pCompTestHwVer->std.memoryType == 0x8Eu) ? w77t25nw_revB : w77q25nw_revB;
            }
            else if (pCompTestHwVer->info.revision >= COMP_TEST_3_FLASH_REVISION_D)
            {
                *target = (pCompTestHwVer->std.memoryType == 0x8Eu) ? w77t25nw_revD : w77q25nw_revD;
            }
            else
            {
                status = COMP_TEST_STATUS__NOT_SUPPORTED;
            }
            break;
        case MAKE32B(0x35u, COMP_TEST_FLASH_SIZE_64Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            *target = (pCompTestHwVer->std.memoryType == 0x8Eu) ? w77t64nw_revA : w77q64nw_revA;
            break;
        case MAKE32B(0x35u, COMP_TEST_FLASH_SIZE_256Mb, (uint8_t)COMP_TEST_FLASH_VOLTAGE_1_8V, 0u):
            *target = (pCompTestHwVer->std.memoryType == 0x8Eu) ? w77t25nw1_revA : w77q25nw1_revA;
            break;
        default:
            status = COMP_TEST_STATUS__NOT_SUPPORTED;
            break;
    }

    return status;
}

void PrintTargetDetected_L(uint32_t target)
{
    switch(target)
    {
    case w77q128jw_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q128JW");
        break;
    case w77q64jw_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q64JW");
        break;
    case w77q128jv_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q128JV");
        break;
    case w77q64jv_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q64JV");
        break;
    case w77q32jw_revB:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q32JW");
        break;
    case w77q25nw_revB:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q25NW");
        break;
    case w77t25nw_revB:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77T25NW");
        break;
    case w77q25nw_revD:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q25NW");
        break;
    case w77t25nw_revD:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77T25NW");
        break;
    case w77q64nw_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q64NW");
        break;
    case w77t64nw_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77T64NW");
        break;
    case w77q25nw1_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77Q25NW");
        break;
    case w77t25nw1_revA:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Detected Target is W77T25NW");
        break;
    default:
        break;
    }
}

/************************************************************************************************************
 * @brief       This function reads Dummy Cycles CR, modifies dummy cycles value and writes CR\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_SetDummy(QLIB_BUS_MODE_T busMode, uint8_t dummy)
{
    uint8_t crValue;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ReadCR(busMode,CR_ADDR_DUMMY_CYCLES,&crValue));
    SET_VAR_FIELD(crValue, CR_DUMMY_FIELD, dummy,uint8_t);
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WriteCR(busMode,CR_ADDR_DUMMY_CYCLES,crValue));
    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function reads SPI Bus Modes CR modifies DQS_EN bit and writes CR\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_SetDqsEnable(QLIB_BUS_MODE_T busMode, bool dqsEn)
{
    uint8_t crValue;
    uint8_t dqsEnValue;

    dqsEnValue = (dqsEn == true)? 1 : 0;
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ReadCR(busMode,CR_ADDR_SPI_BUS_MODES,&crValue));
    SET_VAR_FIELD(crValue, CR_DQS_EN_FIELD, dqsEnValue,uint8_t);
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WriteCR(busMode,CR_ADDR_SPI_BUS_MODES,crValue));
    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function reads SPI Bus Modes CR and returns status of DQS_EN bit \n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_GetDqsEnable(QLIB_BUS_MODE_T busMode, bool *dqsEn)
{
    uint8_t crValue;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ReadCR(busMode,CR_ADDR_SPI_BUS_MODES,&crValue));
    *dqsEn = READ_VAR_FIELD(crValue, CR_DQS_EN_FIELD);

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function reads SR2 and checks QE bit status\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_GetQeEnable(QLIB_BUS_MODE_T busMode, uint8_t *qeFlag)
{
    uint8_t sr2Value;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_Read_SR2(busMode,&sr2Value));
    *qeFlag = READ_VAR_FIELD(sr2Value, SR2_QE_FIELD);
    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function reads SR2 modifies QE bit and programs modified SR2 \n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_SetQeEnable(QLIB_BUS_MODE_T busMode)
{
    uint8_t sr2Value;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_Read_SR2(busMode,&sr2Value));
    SET_VAR_FIELD(sr2Value, SR2_QE_FIELD, 1,uint8_t);
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_Write_SR2(busMode,sr2Value));
    return COMP_TEST_STATUS_OK;
}
