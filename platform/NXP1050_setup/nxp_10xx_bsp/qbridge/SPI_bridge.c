/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       SPI_bridge.h
* @brief      This file includes qlib to nxp bridge definitions for SPI on NXP 10xx device
*
* ### project W77Q
*
************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "SPI_ops.h"
#include "fsl_flexspi.h"
#include "fsl_debug_console.h"
#include "fsl_iomuxc.h"
#include "SPI_bridge.h"
#ifdef Q2_GPT_ACCU
#include "fsl_gpt.h"
#endif
#include "SPI_bridge.h"
#if !defined(QLIB_NO_DIRECT_FLASH_ACCESS)
#include "qlib.h"
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                               DEFINITIONS                                               */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define LUT_DYNAMIC_SEQUENCE_INDEX                    (15)
#define LUT_DYNAMIC_SEQUENCE_SIZE                     (4)

#define FLEXSPI_LUT_SEQ_SINGL_INST(cmd0, pad0, op0) FLEXSPI_LUT_SEQ(cmd0, pad0, op0, 0, 0, 0);
#define FLEXSPI_LUT_SEQ_MAKE_LINE(inst0, inst1) ((inst0) | ((inst1) << FLEXSPI_LUT_OPERAND1_SHIFT))

#define SPI_CMD_DTR(flags) (((flags) & QLIB_DTR_MASK) != QLIB_DTR__NO_DTR)
#define SPI_CMD_IS_SECURE(cmd, format, flags)    ((W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP0, format, SPI_CMD_DTR(flags) ? 1u : 0u) == cmd) ||  (W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP1, format, SPI_CMD_DTR(flags) ? 1u : 0u) == cmd) ||  (W77Q_SEC_INST__MAKE(W77Q_SEC_INST__OP2, format, SPI_CMD_DTR(flags) ? 1u : 0u) == cmd))

#ifdef QLIB_SUPPORT_QPI
#ifdef QLIB_SUPPORT_OPI
#define QLIB_BUS_MODE_IS_SPI(format) (format != QLIB_BUS_MODE_4_4_4 && format != QLIB_BUS_MODE_8_8_8)
#else
#define QLIB_BUS_MODE_IS_SPI(format) (format != QLIB_BUS_MODE_4_4_4)
#endif
#else
#ifdef QLIB_SUPPORT_OPI
#define QLIB_BUS_MODE_IS_SPI(format) (format != QLIB_BUS_MODE_8_8_8)
#else
#define QLIB_BUS_MODE_IS_SPI(format) (1)
#endif
#endif

#define FLEXSPI_SPI_CMD_PHASE                         (0)
#define FLEXSPI_SPI_ADDR_PHASE                        (1)
#define FLEXSPI_SPI_DATA_PHASE                        (2)

#define QLIB_SPI_FLAGS__MODE_BIT_SHIFT (16u)
#define QLIB_SPI_FLAGS__MODE_ENABLE    (1u << QLIB_SPI_FLAGS__MODE_BIT_SHIFT)
#define QLIB_SPI_FLAGS__MODE_SIZE_MASK (0x7)

#define QLIB_SPI_FLAGS__GET_MODE_BITS_SIZE(flags) (!(flags & QLIB_SPI_FLAGS__MODE_ENABLE)) ? 0 : \
                            (((flags >> (QLIB_SPI_FLAGS__MODE_BIT_SHIFT + 1)) & QLIB_SPI_FLAGS__MODE_SIZE_MASK) + 1)
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 GLOBALS                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
extern uint8_t g_SequenceTable[256];
extern SPI_FREQUENCY_T g_spi_freq;
extern SPI_FREQUENCY_T g_set_spi_freq;
extern bool g_set_dqs_mode;
extern bool g_dtr_mode;
extern bool g_dqs_mode;
extern bool g_dqs_pol;

extern bool g_spi_cs_select;

extern QLIB_BUS_MODE_T g_LUT_format;
extern bool g_LUT_DTR;

volatile uint32_t g_SPI_Trns_Cnt = 0;
volatile uint32_t g_LUT_Update_Cnt = 0;
volatile uint32_t g_GPT_Accu = 0;

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                             IMPLEMENTATION                                              */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/


__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_EnableDQS(bool enable)
{
    g_set_dqs_mode = enable;
    return flexspi_enable_dqs(FLEXSPI, g_spi_freq, g_set_dqs_mode, g_dtr_mode);
}

__attribute__ ((section(".ramfunc.$RAM")))
bool PLAT_SPI_GetDqsMode(void)
{
    return flexspi_getDqsMode();
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_GetFreqInHz(uint32_t* freq)
{
    *freq = flexspi_getfreq() * 1000000;
    return 0;
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_SetFreqInHz(uint32_t freq)
{
    g_set_spi_freq = (freq/1000000);
    return flexspi_clock_change(FLEXSPI, g_set_spi_freq, g_dtr_mode, NULL, false);
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_SetChipSelect(uint8_t chipSelect)
{
    if ((bool)chipSelect != g_spi_cs_select)
    {
        if ((bool)chipSelect == false)
        {   // CS0
            IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_15_GPIO1_IO31, 1U); // J14 as GPIO with initial value 1
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_06_FLEXSPIA_SS0_B, 1U); // // L3 as A_SS0_B FlexSPI CS
            IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_06_FLEXSPIA_SS0_B, 0xB0F1U);
            g_spi_cs_select = (bool)chipSelect;
        }
        else
        {  // CS1
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_06_GPIO3_IO06, 1U);  // L3 as GPIO output
            IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_15_FLEXSPIA_SS0_B, 1U); // J14 as A_SS0_B FlexSPI CS
            IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_15_FLEXSPIA_SS0_B, 0xB0F1U);
            g_spi_cs_select = (bool)chipSelect;
        }
    }
    return 0;
}

#define PLAT_QLIB_BUS_MODE(platBusMode) ((QLIB_BUS_MODE_T)platBusMode)

// Convert return value of NXP10xx status to QLIB_STATUS
__attribute__ ((section(".ramfunc.$RAM")))
__inline__ int NXP10xx_writeReadTransaction(PLAT_SPI_FORMAT_T plat_format,
                                 uint32_t        flags,
                                 const uint8_t*  dataOutStream,
                                 uint32_t        cmdSize,
                                 uint32_t        addressSize,
                                 uint32_t        dataOutSize,
                                 uint32_t        dummyCycles,
                                 uint8_t*        dataIn,
                                 uint32_t        dataInSize)
{
    uint32_t ints;
    uint32_t smallData = 0;
    flexspi_transfer_t flashXfer;
    status_t status;
    uint8_t  cmd1 = 0;
    uint8_t  cmd2 = 0;

    uint32_t address = 0;
    const uint8_t* dataOut = NULL;

    uint32_t lutSequence[LUT_DYNAMIC_SEQUENCE_SIZE] = {0};
    uint32_t InstrIndex = 0;
    uint32_t SequenceIndex = 0;
    uint32_t instList[8]; // cmd + addr + (data out / mode) + dummy + data in + stop

    uint32_t dtrCmdMask;
    uint32_t dtrAddrMask;
    uint32_t dtrDataMask;
    uint32_t padsCount[3];

    uint8_t SequenceToUse = LUT_SEQUENCE_TO_USE;

    uint32_t modeCmdType = 0;
    uint8_t  modeValue1 = 0;
    uint8_t  modeValue2 = 0;
    uint32_t modeBitsSize = 0;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Error checking                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    ASSERT_RET(0 == dataInSize || 2 >= dataOutSize, -1);
    ASSERT_RET((cmdSize <= 2), -1);
    ASSERT_RET(addressSize == 0 || addressSize == 3 || addressSize == 4, -1);

#if !defined STRESS_FROM_HOST && defined ENABLE_SPI_COUNTERS
    g_SPI_Trns_Cnt++;
#endif

    if ((dataInSize > 4) && (0 != (dataInSize % 4)))
    {
        dataInSize += 4 - (dataInSize % 4);
    }

    if (cmdSize == 2)
    {
        cmd1 = dataOutStream[0];
        cmd2 = dataOutStream[1];
    }
    else if (cmdSize != 0)
    {
        cmd1 = dataOutStream[0];
    }

    if (addressSize == 4)
    {
        address = MAKE32B(dataOutStream[(cmdSize+3)], dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize]);
    }
    else if (addressSize == 3)
    {
        address = MAKE32B(dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize], 0);
    }

    // Backward compatibility, use dataOutSize == 1 && dataInSize != 0, to indicate mode byte send.
    if ((dataOutSize > 0) && (dataInSize != 0))
    {
        modeBitsSize = QLIB_SPI_FLAGS__GET_MODE_BITS_SIZE(flags);
        // keep Backward compatibility and SSE support, modeBits set multiple of 8 bits with dataOutSize
        modeBitsSize = (modeBitsSize > 0) ? modeBitsSize : (dataOutSize * 8);
    }
    if (dataOutSize > 0)
    {
        if (modeBitsSize > 0)
        {
            modeValue1 = dataOutStream[cmdSize + addressSize];
            if (modeBitsSize > 8)
            {
                modeValue2 = dataOutStream[cmdSize + addressSize + 1];
                dataOut = (dataOutStream + cmdSize + addressSize + 2);
            }
            else
            {
                dataOut = (dataOutStream + cmdSize + addressSize + 1);
            }
        }
        else
        {
            dataOut = (dataOutStream + cmdSize + addressSize);
        }
    }

    // FlexSPI frequency depends on SDR/DTR mode, therefore, any new command that change the SDR/DTR mode,
    // requires internal change of the mode, and the frequency accordingly. For example,
    // when the internal frequency is 332MHz (Max frequency), when sending DTR SPI command,
    // the output signals frequency generated, is automatically divided by 2, 166MHz (Max DTR frequency).
    if ((SPI_CMD_DTR(flags) ? true : false) != g_dtr_mode)
    {
        flexspi_clock_change(FLEXSPI, g_set_spi_freq, (SPI_CMD_DTR(flags) ? true : false), NULL, false);
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Disable interrupts                                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    ints = DisableGlobalIRQ();

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Can't replace MCU fetch command when direct access is enabled
    /*-----------------------------------------------------------------------------------------------------*/
    /* Check if command is already in LUT                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    QLIB_BUS_MODE_T format = PLAT_QLIB_BUS_MODE(plat_format);
    bool cmdDtrEnabled = (SPI_CMD_DTR(flags) ? true : false);
    if ((format != QLIB_BUS_MODE_1_1_1) && SPI_CMD_IS_SECURE(cmd1, format, flags) && ((format != g_LUT_format) || (format == QLIB_BUS_MODE_8_8_8 && cmdDtrEnabled != g_LUT_DTR)))
    {
        // Initialize the correct LUT
        flexspi_secure_LUT_init(format, cmdDtrEnabled);
    }
#if defined QLIB_SUPPORT_QPI || defined QLIB_SUPPORT_OPI
    if (!((((cmd1 == SPI_FLASH_CMD__READ_STATUS_REGISTER_1) || (cmd1 ==  SPI_FLASH_CMD__ENTER_QPI)) && !QLIB_BUS_MODE_IS_SPI(format)) || ((cmd1 == SPI_FLASH_CMD__ENTER_SPI) && (format != QLIB_BUS_MODE_4_4_4))))
#endif
    {
        SequenceToUse = g_SequenceTable[cmd1];

        if (LUT_SEQUENCE_TO_USE != SequenceToUse)
        {
            if ((SEC_CMD_OP1_SDR_SINGLE == cmd1 || SEC_CMD_OP1_SDR_QUAD == cmd1 || SEC_CMD_OP1_SDR_OCTAL == cmd1) && (dataOutSize > 0))
            {
                SequenceToUse += 1;
            }
            goto RUN_CMD;
        }
    }
#endif // QLIB_NO_DIRECT_FLASH_ACCESS
#endif // SPI_LUT_OPTIMIZATION_ENABLED

    /*-----------------------------------------------------------------------------------------------------*/
    /* Prepare the command sequence LUT                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
#ifndef STRESS_FROM_HOST
    g_LUT_Update_Cnt++;
#endif

    dtrCmdMask = (flags & QLIB_SPI_FLAGS__CMD_PHASE_DTR) ? 0x20 : 0x00;
    dtrAddrMask = (flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) ? 0x20 : 0x00;
    dtrDataMask = (flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) ? 0x20 : 0x00;

    switch (plat_format)
    {
        case PLAT_SPI_FORMAT_1_1_1:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_1PAD;
            break;
#ifdef QLIB_SUPPORT_DUAL_SPI
        case PLAT_SPI_FORMAT_1_1_2:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_2PAD;
            break;
        case PLAT_SPI_FORMAT_1_2_2:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_2PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_2PAD;
            break;
#endif // #endif QLIB_SUPPORT_DUAL_SPI

        case PLAT_SPI_FORMAT_1_1_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
        case PLAT_SPI_FORMAT_1_4_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
#ifdef QLIB_SUPPORT_QPI
        case PLAT_SPI_FORMAT_4_4_4:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_4PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_4PAD;
            break;
#endif // #endif QLIB_SUPPORT_QPI

        case PLAT_SPI_FORMAT_1_8_8:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_1PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_8PAD;
            break;
#ifdef QLIB_SUPPORT_OPI
        case PLAT_SPI_FORMAT_8_8_8:
            padsCount[FLEXSPI_SPI_CMD_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_ADDR_PHASE] = kFLEXSPI_8PAD;
            padsCount[FLEXSPI_SPI_DATA_PHASE] = kFLEXSPI_8PAD;
            break;
#endif // #endif QLIB_SUPPORT_OPI
        default:
            EnableGlobalIRQ(ints);
            return -1;
    }

    // 1: cmd
    if(0 != cmdSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_SDR | dtrCmdMask, padsCount[FLEXSPI_SPI_CMD_PHASE], cmd1);
        if (cmdSize == 2)
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_SDR | dtrCmdMask, padsCount[FLEXSPI_SPI_CMD_PHASE], cmd2);
        }
    }

    // 2: addr
    if (0 != addressSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_RADDR_SDR | dtrAddrMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], addressSize * 8);
    }

    // write
    if ((0 != dataOutSize) && !(dataOutSize == 1 && dataInSize != 0)) // to support 77h
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_WRITE_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], 0); // no operands
    }

    // mode bits
    if (modeBitsSize > 0)
    {
        // Verify Mode Command type MODE8/4/2/1 is not lower then padsCount kFLEXSPI_8PAD/4PAD/2PAD/1PAD.
        // for example, if pad is set to kFLEXSPI_8PAD, mode type must be MODE8, and not lower (MODE4/2/1).
        // Mode kFLEXSPI_Command_MODE8/4/2/1 defines the amount of mode bits to send, and not the pad count.
        if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_8PAD) || (modeBitsSize == 8) || (modeBitsSize == 16))
        {
            modeCmdType = kFLEXSPI_Command_MODE8_SDR;
        }
        else if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_4PAD) || (modeBitsSize == 4))
        {
            modeCmdType = kFLEXSPI_Command_MODE4_SDR;
        }
        else if ((padsCount[FLEXSPI_SPI_ADDR_PHASE] == kFLEXSPI_2PAD) || (modeBitsSize == 2))
        {
            modeCmdType = kFLEXSPI_Command_MODE2_SDR;
        }
        else
        {
            modeCmdType = kFLEXSPI_Command_MODE1_SDR;
        }

        if (modeBitsSize <= 8)
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);

            // when sending mode bits, dummy cycles count is reduced.
            // in DTR mode, if mode bits count is lower than data/dummy bus pin count, only rising edge is sent,
            // hence, need to duplicate mode bits data on falling edge.
            if (((flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) != 0) && (modeBitsSize < (uint32_t)(1 << (padsCount[FLEXSPI_SPI_ADDR_PHASE]+1))))
            {
                instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                        modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);
            }
        }
        else
        {
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                            modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue1);
            instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                            modeCmdType | dtrDataMask, padsCount[FLEXSPI_SPI_ADDR_PHASE], modeValue2);
        }
    }

    // dummy clocks
    if (0 != dummyCycles)
    {
        // In case of DTR mode, multiply dummy cycles number
        if ((flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) != 0)
        {
            dummyCycles = (2 * dummyCycles);
        }
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_DUMMY_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], dummyCycles);
    }

    // Read
    if (0 != dataInSize)
    {
        instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(
                kFLEXSPI_Command_READ_SDR | dtrDataMask, padsCount[FLEXSPI_SPI_DATA_PHASE], 0x04); // no operands
    }

    // Stop instruction
    instList[InstrIndex++] = FLEXSPI_LUT_SEQ_SINGL_INST(kFLEXSPI_Command_STOP, 0, 0);

    for(SequenceIndex = 0; SequenceIndex < InstrIndex / 2; SequenceIndex++)
    {
        lutSequence[SequenceIndex] = FLEXSPI_LUT_SEQ_MAKE_LINE(
                instList[SequenceIndex*2], instList[1 + SequenceIndex*2]);
    }

    // Update command LUT table
    FLEXSPI_UpdateLUT(FLEXSPI, LUT_DYNAMIC_SEQUENCE_INDEX * 4, lutSequence, LUT_DYNAMIC_SEQUENCE_SIZE);

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Unused tag
RUN_CMD:
#endif // QLIB_NO_DIRECT_FLASH_ACCESS
#endif // SPI_LUT_OPTIMIZATION_ENABLED
    /*-----------------------------------------------------------------------------------------------------*/
    /* Prepare command in structure                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = SequenceToUse;

    flashXfer.deviceAddress = (0 != addressSize) ? address : 0;

    if (0 != dataInSize)
    {
        flashXfer.cmdType = kFLEXSPI_Read;
        // Support data size smaller than 32bit using temp var, to prevent user buffer overflow
        flashXfer.data = (dataInSize < sizeof(uint32_t)) ? &smallData : (uint32_t*)dataIn;
        flashXfer.dataSize = dataInSize;
    }
    else
    {
        flashXfer.cmdType = (0 != dataOutSize) ? kFLEXSPI_Write : kFLEXSPI_Command;
        flashXfer.data =  (0 != dataOutSize) ? (uint32_t*)dataOut : 0;
        flashXfer.dataSize =  (0 != dataOutSize) ? dataOutSize : 0;
    }

#ifdef Q2_GPT_ACCU
    if(g_GPT_Accu)
    {
        GPT_StartTimer(GPT2);
    }
#endif

    // execute the command
    status = FLEXSPI_TransferBlocking(FLEXSPI, &flashXfer);

#ifdef Q2_GPT_ACCU
    if(g_GPT_Accu)
    {
        GPT_StopTimer(GPT2);
    }
#endif

    if (status != kStatus_Success)
    {
        EnableGlobalIRQ(ints);
        return status;
    }

    if (flashXfer.cmdType == kFLEXSPI_Read)
    {
        if (0 != flashXfer.dataSize && flashXfer.dataSize < sizeof(uint32_t)) //&& flashXfer.dataSize == dataInSize)
        {
#ifdef WINBOND_DEBUG
        // avoid using memcpy(dataIn, &smallData, dataInSize) since memcpy is located on flash
#endif
            dataIn[0] = ((uint8_t*)&smallData)[0];
            if (dataInSize > 1)
            {
                dataIn[1] = ((uint8_t*)&smallData)[1];
            }
            if (dataInSize > 2)
            {
                dataIn[2] = ((uint8_t*)&smallData)[2];
            }
        }
    }

    EnableGlobalIRQ(ints);
    return 0;
}

#if !defined(QLIB_NO_DIRECT_FLASH_ACCESS)
__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_SetFetchCmd(const PLAT_SPI_T* spiIf)
{
    uint32_t memoryFetchCommand[4] = { 0 };
    uint32_t addrBits = (true == spiIf->addrMode3Bytes) ? 24 : 32;
#ifdef QLIB_SUPPORT_QPI
    if (spiIf->mode == PLAT_SPI_FORMAT_4_4_4)
    {
        if (spiIf->dtr == PLAT_SPI_DTR__ADDR_DATA)
        {
            memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_4PAD, SPI_FLASH_CMD__READ_FAST_DTR__4_4_4, RADDR_DDR, FLEXSPI_4PAD, addrBits);
            memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_4PAD, 2 * spiIf->dummyCycles, READ_DDR, FLEXSPI_4PAD, 0);
        }
        else if (spiIf->dtr == PLAT_SPI_DTR__NO_DTR)
        {
            memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_4PAD, SPI_FLASH_CMD__READ_FAST__4_4_4, RADDR_SDR, FLEXSPI_4PAD, addrBits);
            memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, spiIf->dummyCycles, READ_SDR, FLEXSPI_4PAD, 0);
        }
        else
        {
            return -1;
        }
    }
    else
#endif // QLIB_SUPPORT_QPI
#ifdef QLIB_SUPPORT_OPI
        if (spiIf->mode == PLAT_SPI_FORMAT_8_8_8)
        {
            if (spiIf->dtr == PLAT_SPI_DTR__NO_DTR)
            {
                // SOPI
                memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_8PAD, SPI_FLASH_CMD__READ_FAST__8_8_8, RADDR_SDR, FLEXSPI_8PAD, addrBits);
                memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_8PAD, spiIf->dummyCycles, READ_SDR, FLEXSPI_8PAD, 0);
            }
            else if (spiIf->dtr == PLAT_SPI_DTR__ALL)
            {
                // DOPI
                memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_DDR, FLEXSPI_8PAD, SPI_FLASH_CMD__READ_FAST__8_8_8, RADDR_DDR, FLEXSPI_8PAD, addrBits);
                memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_8PAD, 2 * spiIf->dummyCycles, READ_DDR, FLEXSPI_8PAD, 0);
            }
            else
            {
                return -1;
            }
        }
        else
#endif // QLIB_SUPPORT_OPI
            // SPI
            if (spiIf->mode == PLAT_SPI_FORMAT_1_1_1)
            {
                if (spiIf->dtr == PLAT_SPI_DTR__ADDR_DATA)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST_DTR__1_1_1, RADDR_DDR, FLEXSPI_1PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_1PAD, 2 * spiIf->dummyCycles, READ_DDR, FLEXSPI_1PAD, 0);
                }
                else if (spiIf->dtr == PLAT_SPI_DTR__NO_DTR)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST__1_1_1, RADDR_SDR, FLEXSPI_1PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_1PAD, spiIf->dummyCycles, READ_SDR, FLEXSPI_1PAD, 0);
                }
                else
                {
                    return -1;
                }
            }
#ifdef QLIB_SUPPORT_DUAL_SPI
            else if (spiIf->mode == PLAT_SPI_FORMAT_1_2_2)
            {
                if (spiIf->dtr == PLAT_SPI_DTR__ADDR_DATA)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST_DTR__1_2_2, RADDR_DDR, FLEXSPI_2PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_2PAD, 2 * spiIf->dummyCycles, READ_DDR, FLEXSPI_2PAD, 0);
                }
                else if (spiIf->dtr == PLAT_SPI_DTR__NO_DTR)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST__1_2_2, RADDR_SDR, FLEXSPI_2PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_2PAD, spiIf->dummyCycles, READ_SDR, FLEXSPI_2PAD, 0);
                }
                else
                {
                    return -1;
                }
            }
#endif // QLIB_SUPPORT_DUAL_SPI
            else if (spiIf->mode == PLAT_SPI_FORMAT_1_4_4)
            {
                if (spiIf->dtr == PLAT_SPI_DTR__ADDR_DATA)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST_DTR__1_4_4, RADDR_DDR, FLEXSPI_4PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_4PAD, 2 * spiIf->dummyCycles, READ_DDR, FLEXSPI_4PAD, 0);
                }
                else if (spiIf->dtr == PLAT_SPI_DTR__NO_DTR)
                {
                    memoryFetchCommand[0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, SPI_FLASH_CMD__READ_FAST__1_4_4, RADDR_SDR, FLEXSPI_4PAD, addrBits);
                    memoryFetchCommand[1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, spiIf->dummyCycles, READ_SDR, FLEXSPI_4PAD, 0);
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }

    // switch the MCU fetch command
    FLEXSPI_UpdateLUT(FLEXSPI, flexspi_get_fetch_cmd_lut_idx(), memoryFetchCommand, 4);

    // Do software reset
    FLEXSPI_SoftwareReset(FLEXSPI);

    return 0;
}
#endif // #ifdef QLIB_NO_DIRECT_FLASH_ACCESS

