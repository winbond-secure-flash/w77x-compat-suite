/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_main.c
* @brief      This file implements for compatibility tests
*
************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "qlib_platform.h"
#include "common_platform_spi.h"
#include "comp_test_targets.h"
#include "comp_test_main.h"
#include "comp_test_cmd.h"
#include "comp_test_func.h"
#include "string.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                               GLOBALS VSRIABLES                                         */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_GLOBALS_T g_tCompTestGlobals;

#ifdef COMP_TEST_DEBUG
const char* compTestStatusStrings[] = COMP_TEST_STATUS_STR_TAB;
#endif

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
#define SIZE_BY_DEV_ID_TO_SIZE_IN_BYTES(sizeByDevId) ((uint32_t)(((uint32_t)(1))<<(((uint32_t)sizeByDevId) + 1)))

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

static COMP_TEST_STATUS_T TransportCommandsTest(void);
static COMP_TEST_STATUS_T CompareData_L(const uint8_t* DataBuf1, const uint8_t* DataBuf2, uint32_t Size);
COMP_TEST_STATUS_T SetFreqByTargetFlash(uint32_t dieType, QLIB_BUS_MODE_T busmode, bool dqsMode);


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       FUNCTIONS IMPLEMENTATION                                          */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/************************************************************************************************************
 * @brief       This function initializes compatibility tests
 *
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T CompTestInit(void)
{
	COMP_TEST_HW_VER_T tCompTestHwVer;
	COMP_TEST_REG_SSR_T tSsrReg;
    ACTR_T ActrReg;
    uint8_t actrOctSupported;

	COMP_TEST_STATUS_RET_CHECK(GetVersion(&tCompTestHwVer));

	g_tCompTestGlobals.flashSizeInBytes = SIZE_BY_DEV_ID_TO_SIZE_IN_BYTES(tCompTestHwVer.info.flashSize);
	g_tCompTestGlobals.dieType = (tCompTestHwVer.info.dieType);

	if(tCompTestHwVer.info.flashSize == COMP_TEST_FLASH_SIZE_UNKNOWN)
	{
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, "Flash Size Unknown");
        return COMP_TEST_STATUS_FAIL;
	}

    uint8_t qeBit;
    COMP_TEST_GetQeEnable(QLIB_BUS_MODE_1_1_1, &qeBit);
    if(qeBit != 1)
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, "Quad bus mode is not supported");
        return COMP_TEST_STATUS_FAIL; // Quad is not enabled
    }

    if((tCompTestHwVer.info.dieType & q2_mcd_targets)!= 0)
	{
	    g_tCompTestGlobals.ssrNumOfDummyClocks = 32;
	    g_tCompTestGlobals.readNumOfDummyClocks = 8;
	}
	else
	{
        g_tCompTestGlobals.ssrNumOfDummyClocks = 8;
        g_tCompTestGlobals.readNumOfDummyClocks = 8;
        COMP_TEST_Exit_4BA(QLIB_BUS_MODE_1_1_1);
	}
    g_tCompTestGlobals.addressSize = ADDRESS_SIZE_3BA;

    if((tCompTestHwVer.info.dieType & q3_octal_support_targets)!= 0)
    {
        g_tCompTestGlobals.octalSupport = true;
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Octal bus mode is supported");
    }
    else
    {
        g_tCompTestGlobals.octalSupport = false;
        if((tCompTestHwVer.info.dieType == w77q25nw1_revA)||(tCompTestHwVer.info.dieType == w77t25nw1_revA))
        {
            GetActrRegister(&ActrReg);
            actrOctSupported = (uint8_t)READ_VAR_FIELD(ActrReg, REG_ACTR_GS_OCTAL);
            if(actrOctSupported== 1)
            {
                g_tCompTestGlobals.octalSupport = true;
                COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Octal bus mode is supported");
            }
            else
            {
                g_tCompTestGlobals.octalSupport = false;
                COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Octal bus mode is not supported");
            }
        }
    }




    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Read SSR register");
    COMP_TEST_GetSsrOpcode(QLIB_BUS_MODE_1_1_1, &tSsrReg);
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "SSR = 0x%08X",tSsrReg.asUint);
    if ((tSsrReg.asUint != 0)&&(tSsrReg.asUint != 0xFFFFFFFF)&&(tSsrReg.asStruct.STATE < COMP_TEST_DUT_STATE_UNDEFINED_MODE))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"SSR Read passed");
    }
    else
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"SSR Error");
    }

    return COMP_TEST_STATUS_OK;
}


/************************************************************************************************************
 * @brief       This function sets frequency according to flash target maximum frequency
 *              and modifies dummy cycle settings if needed
 *              @param[in]       dieType - Target type
 *              @param[in]       busmode - SPI Bus mode
 *              @param[in]       dqsMode - DQS Enabled
 *
 * @return      Success Status
************************************************************************************************************/
COMP_TEST_STATUS_T SetFreqByTargetFlash(uint32_t dieType, QLIB_BUS_MODE_T busmode, bool dqsMode)
{

    uint32_t freq, readFreq = 0;
    bool readDqsMode;
    uint8_t dummyCyclesValue;

    freq = 133 * 1000000;
    dummyCyclesValue = 8;

    switch(dieType)
    {
        case w77q32jw_revB:
        case w77q128jw_revA:
        case w77q128jv_revA:
        case w77q64jw_revA:
        case w77q64jv_revA:
            freq = 133 * 1000000;
            if(busmode >=QLIB_BUS_MODE_1_4_4)
                dummyCyclesValue = 6;
            break;
        case w77q25nw_revB: //Q3 Quad support
        case w77q25nw_revD:
        case w77q64nw_revA: //Q3.1 Quad support
            freq = 166 * 1000000;
            if(busmode >=QLIB_BUS_MODE_1_4_4)
                dummyCyclesValue = 14;
            break;
        case w77t25nw_revB: // Q3 Octal support
        case w77t25nw_revD:
            if(busmode == QLIB_BUS_MODE_1_4_4)
            {
                freq = 166*1000000;
                dummyCyclesValue = 16;
            }
            if(busmode >= QLIB_BUS_MODE_1_8_8)
            {
                freq = 200*1000000;
                dummyCyclesValue = 20;
            }
            break;
        case w77t64nw_revA:  //Q3.1 Octal support
        case w77q25nw1_revA: //Q3.1 Octal support
        case w77t25nw1_revA: //Q3.1 Octal support
        {
            if(busmode == QLIB_BUS_MODE_1_4_4)
            {
                freq = 166*1000000;
                dummyCyclesValue = 20;
            }
            if((busmode >= QLIB_BUS_MODE_1_8_8)&&(dqsMode == false))
            {
                freq = 166*1000000;
                dummyCyclesValue = 20;
            }
            if((busmode >= QLIB_BUS_MODE_1_8_8)&&(dqsMode == true))
            {
                freq = 200*1000000;
                dummyCyclesValue = 24;
            }
            break;
        }
        default:
            break;
    }

    // Enable DQS if needed
    if (0 != (dieType & all_q3_targets))
    {
        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_SetDqsEnable(QLIB_BUS_MODE_1_1_1,dqsMode));
        PLAT_SPI_EnableDQS(dqsMode);

        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_GetDqsEnable(QLIB_BUS_MODE_1_1_1,&readDqsMode));
        if(readDqsMode != dqsMode)
        {
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Couldn't change Platform DQS settings! DQS mode is %s", ((readDqsMode)?"enabled":"disabled"));
            return COMP_TEST_STATUS_FAIL;
        }
        readDqsMode = PLAT_SPI_GetDqsMode();
        if(readDqsMode != dqsMode)
        {
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Couldn't change Platform DQS settings! DQS mode is %s", ((readDqsMode)?"enabled":"disabled"));
            return COMP_TEST_STATUS_FAIL;
        }
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_DEBUG,"DQS mode is %s", ((readDqsMode)?"enabled":"disabled"));
    }

    // Update Dummy
    if (0 != (dieType & all_q3_targets))
    {
        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_SetDummy(QLIB_BUS_MODE_1_1_1,dummyCyclesValue));
    }
    g_tCompTestGlobals.readNumOfDummyClocks = dummyCyclesValue;
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_DEBUG,"Number of dummy cycles for fast read is %d \n", dummyCyclesValue);

    PLAT_SPI_SetFreqInHz(freq);
    PLAT_SPI_GetFreqInHz(&readFreq);

    if(((readFreq > freq)&&((readFreq - freq) > 10000000))||
       ((freq > readFreq)&&((freq - readFreq) > 10000000)))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Couldn't set platform frequency to required value!\r\nFrequency is set to %d MHz", readFreq/1000000);
        return COMP_TEST_STATUS_FAIL;
    }
    g_tCompTestGlobals.freq = freq;
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Frequency is set to %d MHz", freq/1000000);

    return COMP_TEST_STATUS_OK;
}

/************************************************************************************************************
 * @brief       This function executes the tests
 *
 *
 * @return      Test status
************************************************************************************************************/
COMP_TEST_STATUS_T CompTestExec(void)
{
    COMP_TEST_STATUS_T ret;
     COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Initializing flash device...");
     if(COMP_TEST_STATUS_OK != (ret = CompTestInit()))
     {
         COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, "Error! Device initialization failed!");
         return ret;
     }
     else
     {
         COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Device was initialized successfully");
     }

     COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Transport test started...");
     if(COMP_TEST_STATUS_OK != (ret = TransportCommandsTest()))
     {
         COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, "Error! Transport test failed!");
         return ret;
     }
     else
     {
         COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO, "Transport test was completed successfully");
     }

     return COMP_TEST_STATUS_OK;
}


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                      INTERNAL FUNCTIONS                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


/************************************************************************************************************
 * @brief       This function performs transport tests on SPI Flash\n
 * 				Basic test consist of the following steps: \n
 * 				1. Erase block in a flash memory.\n
 * 				2. Program a block with predefined data.\n
 * 				3. Read the data from this block.\n
 * 				4. Compare this data with predefined data.\n
 * 				\n
 *
 * @return      Test status
************************************************************************************************************/
static COMP_TEST_STATUS_T TransportCommandsTest(void)
{
	uint32_t Address = 0x0;
    uint8_t DataInBuff[TEST_DATA_BUFF_SIZE];
    uint8_t DataOutBuff[TEST_DATA_BUFF_SIZE];
    COMP_TEST_STATUS_T ret = COMP_TEST_STATUS_OK;
    // Prepare Data
    memset(DataInBuff, 0x5A,TEST_DATA_BUFF_SIZE);
    memset(DataOutBuff,0x00,TEST_DATA_BUFF_SIZE);

    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Test Single bus mode: 1s-1s-1s");
    // Modify frequency
    COMP_TEST_STATUS_RET_CHECK(SetFreqByTargetFlash(g_tCompTestGlobals.dieType, QLIB_BUS_MODE_1_1_1, false));

    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute erase sector command");
   	COMP_TEST_STATUS_RET_CHECK(COMP_TEST_EraseSectorCmd(QLIB_BUS_MODE_1_1_1, Address));
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute page program command");
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Program Pattern 0x5A");
   	COMP_TEST_STATUS_RET_CHECK(COMP_TEST_ProgramDataCmd(QLIB_BUS_MODE_1_1_1, Address, DataInBuff, TEST_DATA_BUFF_SIZE));
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute fast read page command");
   	COMP_TEST_STATUS_RET_CHECK(COMP_TEST_FastReadCmd(QLIB_BUS_MODE_1_1_1, Address, DataOutBuff, TEST_DATA_BUFF_SIZE));
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Data Pattern 0x5A");
    if (COMP_TEST_STATUS_OK != CompareData_L(DataInBuff, DataOutBuff, TEST_DATA_BUFF_SIZE))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Data mismatch!");
        ret = COMP_TEST_STATUS_TRANSPORT_SDR_FAIL;
    }else{
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Successfully");
    }


    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Test Quad bus mode: 1s-4s-4s");
    // Modify frequency for Quad
    COMP_TEST_STATUS_RET_CHECK(SetFreqByTargetFlash(g_tCompTestGlobals.dieType, QLIB_BUS_MODE_1_4_4, false));

    memset(DataOutBuff, 0x00, TEST_DATA_BUFF_SIZE);
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute fast read page command");
    COMP_TEST_STATUS_RET_CHECK(COMP_TEST_FastReadCmd(QLIB_BUS_MODE_1_4_4, Address, DataOutBuff, TEST_DATA_BUFF_SIZE));
    COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Data Pattern 0x5A");
	if (COMP_TEST_STATUS_OK != CompareData_L(DataInBuff, DataOutBuff, TEST_DATA_BUFF_SIZE))
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Data mismatch!");
        ret = COMP_TEST_STATUS_TRANSPORT_SDR_FAIL;
    }else{
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Successfully");
    }

    if(g_tCompTestGlobals.octalSupport == true)
    {
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Test Octal bus mode: 1s-8s-8s");
        // Modify frequency for octal
        COMP_TEST_STATUS_RET_CHECK(SetFreqByTargetFlash(g_tCompTestGlobals.dieType, QLIB_BUS_MODE_1_8_8, false));

        memset(DataOutBuff, 0x00, TEST_DATA_BUFF_SIZE);
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute fast read page command");
        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_FastReadCmd(QLIB_BUS_MODE_1_8_8, Address, DataOutBuff, TEST_DATA_BUFF_SIZE));
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Data Pattern 0x5A");
        if (COMP_TEST_STATUS_OK != CompareData_L(DataInBuff, DataOutBuff, TEST_DATA_BUFF_SIZE))
        {
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Data mismatch!");
            ret = COMP_TEST_STATUS_TRANSPORT_SDR_FAIL;
        }else{
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Successfully");
        }
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Test Octal SOPI bus mode: 8s-8s-8s");
        memset(DataOutBuff, 0x00, TEST_DATA_BUFF_SIZE);
        COMP_TEST_EnterOpi(QLIB_BUS_MODE_1_1_1, COMP_TEST_DTR_OFF);
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute fast read page command");
        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_FastReadCmd(QLIB_BUS_MODE_8_8_8, Address, DataOutBuff, TEST_DATA_BUFF_SIZE));
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Data Pattern 0x5A");
        if (COMP_TEST_STATUS_OK != CompareData_L(DataInBuff, DataOutBuff, TEST_DATA_BUFF_SIZE))
        {
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Data mismatch!");
            ret = COMP_TEST_STATUS_TRANSPORT_SDR_FAIL;
        }else{
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Successfully");
        }
        COMP_TEST_EnterSpi(QLIB_BUS_MODE_8_8_8);

        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Test Octal DOPI bus mode: 8d-8d-8d");
        COMP_TEST_STATUS_RET_CHECK(SetFreqByTargetFlash(g_tCompTestGlobals.dieType, QLIB_BUS_MODE_8_8_8, true));
        memset(DataOutBuff, 0x00, TEST_DATA_BUFF_SIZE);

        COMP_TEST_EnterOpi(QLIB_BUS_MODE_1_1_1, COMP_TEST_DTR_ON);
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Execute fast read page command");
        COMP_TEST_STATUS_RET_CHECK(COMP_TEST_FastReadDtrCmd(QLIB_BUS_MODE_8_8_8, Address, DataOutBuff, TEST_DATA_BUFF_SIZE));
        COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Data Pattern 0x5A");
        if (COMP_TEST_STATUS_OK != CompareData_L(DataInBuff, DataOutBuff, TEST_DATA_BUFF_SIZE))
        {
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR,"Error! Data mismatch!");
            ret = COMP_TEST_STATUS_TRANSPORT_SDR_FAIL;
        }else{
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_INFO,"Verify Successfully");
        }
        COMP_TEST_EnterSpi(QLIB_BUS_MODE_8_8_8);
    }

    PLAT_SPI_SetFreqInHz(50000000);
    COMP_TEST_STATUS_RET_CHECK(SetFreqByTargetFlash(g_tCompTestGlobals.dieType, QLIB_BUS_MODE_1_1_1, false));
    return ret;
}

/************************************************************************************************************
 * @brief       This function compares to data buffers of size 'Size'\n
 * @param[in]       DataBuf1   - Pointer to data buffer for comparison
 * @param[in]       DataBuf2   - Pointer to data buffer for comparison
 * @param[in]       Size       - Size of buffers to compare
 *
 * @return      Test status
************************************************************************************************************/
static COMP_TEST_STATUS_T CompareData_L(const uint8_t* DataBuf1, const uint8_t* DataBuf2, uint32_t Size)
{
	uint32_t i;

	for(i=0; i < Size; i++)
	{
		if(DataBuf1[i] != DataBuf2[i])
			return COMP_TEST_STATUS_FAIL;
	}

	return COMP_TEST_STATUS_OK;
}


uint8_t gDataOutStream[(2 + 4 + 8 + FLASH_PAGE_SIZE)]  __attribute__((aligned(8)));
#define QLIB_CMD_SIZE__1   1
#define QLIB_CMD_SIZE__2   2
#define QLIB_BUS_AND_DTR_TO_QLIB_FLAGS(format, dtr) \
    (dtr == QLIB_DTR__NO_DTR) ? QLIB_DTR__NO_DTR : \
    ((format == QLIB_BUS_MODE_1_1_1) || (format == QLIB_BUS_MODE_1_4_4) || (format == QLIB_BUS_MODE_1_8_8) || (format == QLIB_BUS_MODE_4_4_4)) ? \
     QLIB_DTR__ADDR_DATA : \
    (format == QLIB_BUS_MODE_8_8_8) ? QLIB_DTR__ALL : QLIB_DTR__NO_DTR

#define FLAGS_AND_QLIB_BUS_TO_COMMAMD_SIZE(flags, format) \
   ( ((flags & QLIB_SPI_FLAGS__CMD_PHASE_DTR) == QLIB_SPI_FLAGS__CMD_PHASE_DTR)  && \
     (format == QLIB_BUS_MODE_8_8_8) ) ? QLIB_CMD_SIZE__2 : QLIB_CMD_SIZE__1

COMP_TEST_STATUS_T COMP_TEST_WriteReadTransaction(const void*          userData,
                                   QLIB_BUS_MODE_T      format,
                                   bool                 dtr,
                                   uint8_t              cmd,
                                   const uint32_t       address,
                                   const uint32_t 		addressSize,
                                   const uint8_t*       dataOut,
                                   uint32_t             dataOutSize,
                                   uint32_t             dummyCycles,
                                   uint8_t*             dataIn,
                                   uint32_t             dataInSize)
{
    uint32_t        flags;
    uint32_t        cmdSize;
    uint32_t        bufIndex = 0;

    QLIB_BUS_MODE_T qlibSpiFormat = format;

    flags = (uint32_t)QLIB_BUS_AND_DTR_TO_QLIB_FLAGS(qlibSpiFormat, dtr);

    cmdSize = FLAGS_AND_QLIB_BUS_TO_COMMAMD_SIZE(flags, qlibSpiFormat);

    COMP_TEST_ASSERT_RET((cmdSize + addressSize + dataOutSize) <= (2 + 4 + FLASH_PAGE_SIZE), COMP_TEST_STATUS__INVALID_DATA_SIZE);
    if (cmdSize < 2)
    {
        gDataOutStream[bufIndex++] = cmd;
    }
    else
    {
        switch (qlibSpiFormat)
        {
            case QLIB_BUS_MODE_1_1_1:
            case QLIB_BUS_MODE_1_1_2:
            case QLIB_BUS_MODE_1_2_2:
            case QLIB_BUS_MODE_1_1_4:
            case QLIB_BUS_MODE_1_4_4:
            case QLIB_BUS_MODE_1_8_8:
            case QLIB_BUS_MODE_4_4_4:
                return -1;//QLIB_STATUS__INVALID_PARAMETER;
                break;
            case QLIB_BUS_MODE_8_8_8:
                gDataOutStream[bufIndex++] = cmd;
                gDataOutStream[bufIndex++] = cmd;
                break;
            default:
                break;
        }
    }
    if (addressSize > 0)
    {
        gDataOutStream[bufIndex++] = BYTE(address, addressSize - 1);
        gDataOutStream[bufIndex++] = BYTE(address, addressSize - 2);
        gDataOutStream[bufIndex++] = BYTE(address, addressSize - 3);
        if (addressSize == 4)
        {
        gDataOutStream[bufIndex++] = BYTE(address, 0);
        }
    }

    if (dataOutSize > 0)
    {
        memcpy(&gDataOutStream[cmdSize + addressSize], dataOut, dataOutSize);
    }

    return PLAT_SPI_WriteReadTransaction(userData,
        qlibSpiFormat,
        flags,
        gDataOutStream,
        cmdSize,
        addressSize,
        dataOutSize,
        dummyCycles,
        dataIn,
        dataInSize);

}
