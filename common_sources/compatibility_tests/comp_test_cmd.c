/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_cmd.c
* @brief      This file implements flash commands

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
#include "string.h"
//#include "qlib_std_cmds_.h"

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


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       FUNCTIONS IMPLEMENTATION                                          */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/************************************************************************************************************
 * @brief       This function performs Get Jedec ID
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_GetJedecId(uint8_t* JedecIdBuf)
{

    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_1_1, 0, SPI_FLASH_CMD__READ_JEDEC, 0x0, 0, NULL, 0, DUMMY_CYCLES__RD_JEDEC_ID__SINGLE, JedecIdBuf, 3)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    if((JedecIdBuf[0] != SPI_DEVICE_MANUFACTURER_ID))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, "Manufacturer ID mismatch. JedecIdBuf[0] = 0x%2X", JedecIdBuf[0]);
        return COMP_TEST_STATUS_FAIL;
    }

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function performs Get Device ID tests on SPI Flash\n
 *              It issues GET_DEVICE_ID commands in all available SPI bus formats.\n
 *              and compares the value with expected Device ID\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_GetDeviceId(uint8_t* deviceId)
{
    // Single
    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_1_1, 0, SPI_FLASH_CMD__DEVICE_ID, 0x0, 0, NULL, 0, DUMMY_CYCLES__RD_DEVICE_ID_SINGLE, deviceId, 1)))
          return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
* Function:        COMP_TEST_GetSsrOpcode (A0h)
*
* Parameters:      format      - (IN)  SPI IO format (cmd - address - data), i.e: SPI_BUS_MODE_4_4_4
*                  ssr         - (OUT) ssr read from register
* Returns:         0 if no error occurred, COMP_TEST__(ERROR) otherwise
* Side effects:    none
* Description:
*                  This routine read Secure Status Register (SSR)
*                  Support format: 1_1_1 (Single)
************************************************************************************************************/
COMP_TEST_STATUS_T  COMP_TEST_GetSsrOpcode(QLIB_BUS_MODE_T format, COMP_TEST_REG_SSR_T* ssr)
{
    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__OP0_1_1_1, 0, 0, NULL, 0, g_tCompTestGlobals.ssrNumOfDummyClocks, (uint8_t*)(&(ssr->asUint)), SPI_SSR_RESPONSE_COUNT)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        COMP_TEST_WriteBufOpcode (A1h)                                                         */
/*                                                                                                         */
/* Parameters:      format      - (IN)  SPI IO format (cmd - address - data), i.e: SPI_BUS_MODE_4_4_4      */
/*                  ctag        - (IN)  CTAG opcode                                                        */
/* Returns:         0 if no error occurred, COMP_TEST__(ERROR) otherwise                                   */
/* Side effects:    none                                                                                   */
/* Description:                                                                                            */
/*                  This routine writes to output buffer                                                   */
/*                  Support format: 1_1_1 (Single)                                                         */
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_STATUS_T COMP_TEST_WriteBufOpcode(QLIB_BUS_MODE_T format, uint32_t ctag, uint8_t* bufToWrite, uint8_t size)
{

    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__OP1_1_1_1, ctag, 4, bufToWrite, size, 0, NULL, 0)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        COMP_TEST_ReadBufOpcode (A2h)                                                            */
/*                                                                                                         */
/* Parameters:      format      - (IN)  SPI IO format (cmd - address - data), i.e: SPI_BUS_MODE_4_4_4      */
/*                  buf         - (OUT) Buffer for data                                                       */
/*                  size        - (IN)  Num of data bytes to read                                          */
/* Returns:         0 if no error occurred, COMP_TEST__(ERROR) otherwise                                   */
/* Side effects:    none                                                                                   */
/* Description:                                                                                            */
/*                  This routine reads Input Data Buffer                                                   */
/*                  Support format: 1_1_1 (Single)                                                         */
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_STATUS_T COMP_TEST_ReadBufOpcode(QLIB_BUS_MODE_T format, uint8_t *buf, uint32_t size)
{
    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__OP2_1_1_1, 0, 0, NULL, 0, STD_DUMMY_CYCLES_8, buf, size)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function performs write enable flash command on SPI\n
 * @param[in]       busFormat   - SPI Bus format
 *
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_WriteEnable(QLIB_BUS_MODE_T format)
{

    if (format != QLIB_BUS_MODE_4_4_4)
        format = QLIB_BUS_MODE_1_1_1;

    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__WRITE_ENABLE, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function read flash status and perform busy waiting\n
 * @param[in]       busFormat   - SPI Bus format
 *
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_WaitWhileBusy(QLIB_BUS_MODE_T format)
{
    uint8_t StatusReg1;

    if (format != QLIB_BUS_MODE_4_4_4)
        format = QLIB_BUS_MODE_1_1_1;

    do{
        if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__READ_STATUS_REGISTER_1, 0, 0, NULL, 0, 0, (uint8_t*)&StatusReg1, 1))
            return COMP_TEST_STATUS__CONNECTIVITY_ERR;
    }while(0 != (StatusReg1 & 0x01));

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function perform flash sector erase on SPI \n
 * @param[in]       busFormat   - SPI Bus format
 * @param[in]       Address     - Start address for erase operation
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_EraseSectorCmd(QLIB_BUS_MODE_T format, uint32_t Address)
{

    if(Address > g_tCompTestGlobals.flashSizeInBytes)
        return -1; //illegal parameters

    if (format != QLIB_BUS_MODE_4_4_4)
        format = QLIB_BUS_MODE_1_1_1;

    Address = Address & 0xFFFFF000; // Align to the page size , i.e. 4 kBytes

    COMP_TEST_STATUS_RET_CHECK( COMP_TEST_WriteEnable(format));

    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__ERASE_SECTOR);
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__ERASE_SECTOR, Address, g_tCompTestGlobals.addressSize, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WaitWhileBusy(format));

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function perform flash program on SPI \n
 * @param[in]       busFormat   - SPI Bus format
 * @param[in]       Address     - Start address for program operation
 * @param[in]       OutDataBuf  - Pointer to data buffer for programming
 * @param[in]       Size        - Size of data to read
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_ProgramDataCmd(QLIB_BUS_MODE_T format, uint32_t Address, uint8_t* DataBuf, uint32_t BufSize)
{
    COMP_TEST_STATUS_T ret;

    if((Address > g_tCompTestGlobals.flashSizeInBytes)||(BufSize > FLASH_SECT_SIZE))
        return -1; //illegal parameters

    if ((format != QLIB_BUS_MODE_1_4_4)&&(format != QLIB_BUS_MODE_4_4_4))
        format = QLIB_BUS_MODE_1_1_1;

    Address = Address & 0xFFFFFF00; // Align to the page size , i.e. 256 bytes

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WriteEnable(format));

    if ((format == QLIB_BUS_MODE_1_1_1)||(format == QLIB_BUS_MODE_4_4_4))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__PAGE_PROGRAM);
        ret = COMP_TEST_WriteReadTransaction(NULL, format, 0, SPI_FLASH_CMD__PAGE_PROGRAM, Address, g_tCompTestGlobals.addressSize, DataBuf, BufSize, 0, NULL, 0);
    }
    else
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__PAGE_PROGRAM_1_4_4);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_4_4, 0, SPI_FLASH_CMD__PAGE_PROGRAM_1_4_4, Address, g_tCompTestGlobals.addressSize, DataBuf, BufSize, 0, NULL, 0);
     }

    if (ret != COMP_TEST_STATUS_OK)
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_WaitWhileBusy(format));

    return COMP_TEST_STATUS_OK;

}


/************************************************************************************************************
 * @brief       This function perform fast read on SPI using single data transfer rate\n
 * @param[in]       busFormat   - SPI Bus format
 * @param[in]       Address     - Start address to read from
 * @param[out]      OutDataBuf  - Pointer to data buffer to store read data
 * @param[in]       Size        - Size of data to read
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_FastReadCmd(QLIB_BUS_MODE_T busFormat, uint32_t Address, uint8_t* OutDataBuf, uint32_t Size)
{
    COMP_TEST_STATUS_T ret;

    if(Address > g_tCompTestGlobals.flashSizeInBytes)
        return COMP_TEST_STATUS__PARAMETER_OUT_OF_RANGE; //illegal parameters

    switch (busFormat)
    {
    case QLIB_BUS_MODE_1_1_1:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST__1_1_1);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_1_1, 0, SPI_FLASH_CMD__READ_FAST__1_1_1, Address, g_tCompTestGlobals.addressSize, NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    case QLIB_BUS_MODE_1_4_4:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST__1_4_4);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_4_4, 0, SPI_FLASH_CMD__READ_FAST__1_4_4, Address, g_tCompTestGlobals.addressSize, NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    case QLIB_BUS_MODE_1_8_8:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST__1_8_8);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_8_8, 0, SPI_FLASH_CMD__READ_FAST__1_8_8, Address, g_tCompTestGlobals.addressSize, NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    case QLIB_BUS_MODE_8_8_8:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST__8_8_8);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_8_8_8, 0, SPI_FLASH_CMD__READ_FAST__8_8_8, Address, ADDRESS_SIZE_4BA,               NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    default:
        return COMP_TEST_STATUS__INVALID_PARAMETER;
    }

    if (COMP_TEST_STATUS_OK != ret)
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}


/************************************************************************************************************
 * @brief       This function perform fast read on SPI using dual data transfer rate\n
 * @param[in]       busFormat   - SPI Bus format
 * @param[in]       Address     - Start address to read from
 * @param[out]      OutDataBuf  - Pointer to data buffer to store read data
 * @param[in]       Size        - Size of data to read
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_FastReadDtrCmd(QLIB_BUS_MODE_T busFormat, uint32_t Address, uint8_t* OutDataBuf, uint32_t Size)
{
    COMP_TEST_STATUS_T ret;

    if(Address > g_tCompTestGlobals.flashSizeInBytes)
        return COMP_TEST_STATUS__PARAMETER_OUT_OF_RANGE; //illegal parameters

    switch (busFormat)
    {
    case QLIB_BUS_MODE_1_1_1:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST_DTR__1_1_1);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_1_1, true, SPI_FLASH_CMD__READ_FAST_DTR__1_1_1, Address, g_tCompTestGlobals.addressSize, NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
   case QLIB_BUS_MODE_1_8_8:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST_DTR__1_8_8);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_1_8_8, true, SPI_FLASH_CMD__READ_FAST_DTR__1_8_8, Address, g_tCompTestGlobals.addressSize, NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    case QLIB_BUS_MODE_8_8_8:
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Using opcode 0x%02X",SPI_FLASH_CMD__READ_FAST_DTR__8_8_8);
        ret = COMP_TEST_WriteReadTransaction(NULL, QLIB_BUS_MODE_8_8_8, true, SPI_FLASH_CMD__READ_FAST_DTR__8_8_8,     Address, ADDRESS_SIZE_4BA,               NULL, 0, g_tCompTestGlobals.readNumOfDummyClocks, OutDataBuf, Size);
        break;
    default:
        return COMP_TEST_STATUS__INVALID_PARAMETER;
    }

    if (COMP_TEST_STATUS_OK != ret)
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function executes SPI command to enter QPI mode\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_EnterQpi(QLIB_BUS_MODE_T busMode)
{
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__ENTER_QPI, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function executes SPI command to enter OPI mode\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_EnterOpi(QLIB_BUS_MODE_T busMode, bool dtrMode)
{
    uint8_t mode;
    mode = (dtrMode == COMP_TEST_DTR_ON)? (0x82) : (0x81) ;
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__ENTER_OPI, 0, 0, &mode, 1, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function executes SPI command to exit QPI/OPI mode\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_EnterSpi(QLIB_BUS_MODE_T busMode)
{
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__ENTER_SPI, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

 return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function sends SW reset sequence on SPI\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_ResetFlash(QLIB_BUS_MODE_T busMode)
{
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__RESET_ENABLE, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__RESET_DEVICE, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;
    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function sends command to exit 4 BA mode\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_Exit_4BA(QLIB_BUS_MODE_T busMode)
{
    if (COMP_TEST_STATUS_OK != COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__4_BYTE_ADDRESS_MODE_EXIT, 0, 0, NULL, 0, 0, NULL, 0))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function sends command to write CR register\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_WriteCR(QLIB_BUS_MODE_T busMode, uint8_t addr, uint8_t value)
{
    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteEnable(busMode)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__WRITE_CR, addr, g_tCompTestGlobals.addressSize, &value, 1, 0, NULL, 0)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function sends command to read CR register\n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_ReadCR(QLIB_BUS_MODE_T busMode, uint8_t addr, uint8_t *value)
{
    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__READ_CR, addr, g_tCompTestGlobals.addressSize, NULL, 0, SPI_FLASH_DUMMY_CYCLES__CR, value, 1)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function reads SR2 register \n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_Read_SR2(QLIB_BUS_MODE_T busMode, uint8_t *StatusReg2)
{

    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__READ_STATUS_REGISTER_2, 0, 0, NULL, 0, 0, (uint8_t*)StatusReg2, 1)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
/************************************************************************************************************
 * @brief       This function writes SR2 register \n
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T COMP_TEST_Write_SR2(QLIB_BUS_MODE_T busMode, uint8_t StatusReg2)
{

    if (COMP_TEST_STATUS_OK != (COMP_TEST_WriteReadTransaction(NULL, busMode, 0, SPI_FLASH_CMD__WRITE_STATUS_REGISTER_2, 0, 0, (uint8_t*)&StatusReg2, 1, 0, NULL, 0)))
        return COMP_TEST_STATUS__CONNECTIVITY_ERR;

    return COMP_TEST_STATUS_OK;
}
