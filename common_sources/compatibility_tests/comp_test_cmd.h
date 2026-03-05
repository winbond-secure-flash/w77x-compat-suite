/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2025 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_cmd.h
* @brief      This file includes definitions for commands implementations
*
*
************************************************************************************************************/
#ifndef __COMP_TEST_CMD_H__
#define __COMP_TEST_CMD_H__
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 DEFINES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_DEVICE_MANUFACTURER_ID (0xEF)
#define SPI_DEVICE_ID              (0x15)

#define DUMMY_CYCLES__RD_JEDEC_ID__SINGLE (0)
#define DUMMY_CYCLES__RD_DEVICE_ID_SINGLE (24)
#define STD_DUMMY_CYCLES_8                (8)

#define ADDRESS_SIZE_3BA (3)
#define ADDRESS_SIZE_4BA (4)

#define COMP_TEST_DTR_OFF (false)
#define COMP_TEST_DTR_ON  (true)
#define FLASH_PAGE_SIZE   (256)

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/* SSR bit-field structure                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    uint32_t BUSY : 1U;
    uint32_t RESERVED_0 : 1U;
    uint32_t ERR : 1U;
    uint32_t RESERVED_1 : 1U;

    uint32_t SES_READY : 1U;
    uint32_t RESP_READY : 1U;
    uint32_t POR : 1U;
    uint32_t FB_REMAP : 1U;

    uint32_t AWDT_EXP : 1U;
    uint32_t RESERVED_2 : 1U;
    uint32_t SES_ERR_S : 1U;
    uint32_t SPIF_ERR : 1U;

    uint32_t INTG_ERR_S : 1U;
    uint32_t AUTH_ERR_S : 1U;
    uint32_t PRIV_ERR_S : 1U;
    uint32_t IGNORE_ERR_S : 1U;

    uint32_t SYS_ERR_S : 1U;
    uint32_t FLASH_ERR_S : 1U;
    uint32_t RESERVED_3 : 1U;
    uint32_t MC_ERR : 1U;

    uint32_t MC_MAINT : 2U;
    uint32_t SUSPEND_E : 1U;
    uint32_t SUSPEND_W : 1U;

    uint32_t STATE : 3U;
    uint32_t FULL_PRIV : 1U;

    uint32_t KID : 4U;
} COMP_TEST_REG_SSR_STRUCT_T;

/*---------------------------------------------------------------------------------------------------------*/
/* SSR bitfield-uint32_t union                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
typedef union
{
    uint32_t                   asUint;
    COMP_TEST_REG_SSR_STRUCT_T asStruct;
} COMP_TEST_REG_SSR_T;

typedef enum COMP_TEST_DUT_STATE_MODE {
    COMP_TEST_DUT_STATE_RESET_INPROGRESS_MODE = 0,
    COMP_TEST_DUT_STATE_REPAIR_MODE = 1,
    COMP_TEST_DUT_STATE_WORK_MODE = 2,
    COMP_TEST_DUT_STATE_LOCK_MODE = 4,
    COMP_TEST_DUT_STATE_TEST_MODE = 5,
    COMP_TEST_DUT_STATE_UNDEFINED_MODE,
}COMP_TEST_DUT_STATE_MODE;


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       FUNCTIONS DECLARATIONS                                            */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
COMP_TEST_STATUS_T COMP_TEST_GetJedecId(uint8_t* JedecIdBuf);
COMP_TEST_STATUS_T COMP_TEST_GetDeviceId(uint8_t* deviceId);
COMP_TEST_STATUS_T COMP_TEST_GetSsrOpcode(QLIB_BUS_MODE_T format, COMP_TEST_REG_SSR_T* ssr);
COMP_TEST_STATUS_T COMP_TEST_WriteBufOpcode(QLIB_BUS_MODE_T format, uint32_t ctag, uint8_t* bufToWrite, uint8_t size);
COMP_TEST_STATUS_T COMP_TEST_ReadBufOpcode(QLIB_BUS_MODE_T format, uint8_t *buf, uint32_t size);
COMP_TEST_STATUS_T COMP_TEST_WriteEnable(QLIB_BUS_MODE_T format);
COMP_TEST_STATUS_T COMP_TEST_WaitWhileBusy(QLIB_BUS_MODE_T format);
COMP_TEST_STATUS_T COMP_TEST_EraseSectorCmd(QLIB_BUS_MODE_T format, uint32_t Address);
COMP_TEST_STATUS_T COMP_TEST_ProgramDataCmd(QLIB_BUS_MODE_T format, uint32_t Address, uint8_t* DataBuf, uint32_t BufSize);
COMP_TEST_STATUS_T COMP_TEST_FastReadCmd(QLIB_BUS_MODE_T busFormat, uint32_t Address, uint8_t* OutDataBuf, uint32_t Size);
COMP_TEST_STATUS_T COMP_TEST_FastReadDtrCmd(QLIB_BUS_MODE_T busFormat, uint32_t Address, uint8_t* OutDataBuf, uint32_t Size);
COMP_TEST_STATUS_T COMP_TEST_EnterQpi(QLIB_BUS_MODE_T busMode);
COMP_TEST_STATUS_T COMP_TEST_EnterOpi(QLIB_BUS_MODE_T busMode, bool dtrMode);
COMP_TEST_STATUS_T COMP_TEST_EnterSpi(QLIB_BUS_MODE_T busMode);
COMP_TEST_STATUS_T COMP_TEST_ResetFlash(QLIB_BUS_MODE_T busMode);
COMP_TEST_STATUS_T COMP_TEST_Exit_4BA(QLIB_BUS_MODE_T busMode);
COMP_TEST_STATUS_T COMP_TEST_WriteCR(QLIB_BUS_MODE_T busMode, uint8_t addr, uint8_t value);
COMP_TEST_STATUS_T COMP_TEST_ReadCR(QLIB_BUS_MODE_T busMode, uint8_t addr, uint8_t *value);
COMP_TEST_STATUS_T COMP_TEST_Read_SR2(QLIB_BUS_MODE_T busMode, uint8_t *StatusReg2);
COMP_TEST_STATUS_T COMP_TEST_Write_SR2(QLIB_BUS_MODE_T busMode, uint8_t StatusReg2);

#endif // __COMP_TEST_CMD_H__
