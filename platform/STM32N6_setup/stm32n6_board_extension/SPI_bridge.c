/************************************************************************************************************
* @copyright  Copyright (c) 2026 by Winbond Electronics Corporation . All rights reserved
*
* @file       SPI_bridge.c
* @brief      This file contains implementation for SPI bridge of XSPI flash interface on STM32N6 device
*
************************************************************************************************************/

#include <string.h>
#include "SPI_bridge.h"
#include "qlib_platform.h"
#include "common_platform_capabilities.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define STM32N6_XSPI_HIGH_ADDR_WORKAROUND   // To overcome XSPI bug on STM32N6 MCU which limit address up to 28bits

/*---------------------------------------------------------------------------------------------------------*/
/* Make 32-bit value from 4 bytes                                                                          */
/*                                                                                                         */
/* MAKE32B operation example:                                                                              */
/* Four 8bit sources: <byte0>, <byte1>, <byte2>, <byte3>                                                   */
/* 32bit result:      <byte3><byte2><byte1><byte0>                                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define MAKE32B(b0, b1, b2, b3) \
    ((uint32_t)( (uint32_t)(b0)        | \
                ((uint32_t)(b1) << 8)  | \
                ((uint32_t)(b2) << 16) | \
                ((uint32_t)(b3) << 24)))


#define FREQ_100_MHZ   (100000000)
#define FREQ_200_MHZ   (200000000)



/*******************************************************************************
 * Variables
 ******************************************************************************/

extern XSPI_HandleTypeDef hxspi2;     // XSPI handle defined in main.c, used for SPI transactions with flash
static XSPI_HandleTypeDef *hxspi = &hxspi2;

static int g_dqs_mode = HAL_XSPI_DQS_DISABLE;

static uint32_t g_spi_freq = FREQ_100_MHZ;  // STM32 initialized XSPI flash frequency



/*******************************************************************************
 * Code
 ******************************************************************************/

int PLAT_SPI_EnableDQS(bool enable)
{
    return STM32_enable_dqs(enable);
}

bool PLAT_SPI_GetDqsMode(void)
{
    return g_dqs_mode;
}

// XSPI2 Set Frequency [MHz]
// Parameters:
//   frequencyMhz Range: 10-400
int XSPI_SetFrequency(XSPI_HandleTypeDef *hxspi, uint32_t frequencyMhz)
{
#if 0 // TODO: implement for STM32N6

#endif
    return 0;
}

int PLAT_SPI_GetFreqInHz(uint32_t* freq)
{
    *freq = g_spi_freq * 1000000;
    return 0;
}

int PLAT_SPI_SetFreqInHz(uint32_t freq)
{
    g_spi_freq = (freq/1000000);
    return XSPI_SetFrequency(&hxspi2, g_spi_freq);
}

int PLAT_GetCapabilities(PLAT_SPI_HW_CAPABILITIES_T* hwCapabilities)
{
    memset(hwCapabilities, 0, sizeof(PLAT_SPI_HW_CAPABILITIES_T));
    hwCapabilities->dqs = true;
    hwCapabilities->dtr = true;
    hwCapabilities->hw_flash_voltage_set = false;
    hwCapabilities->hw_gpio = false;
    hwCapabilities->hw_leds = true;
    hwCapabilities->hw_reset_to_flash = false;
    hwCapabilities->reset_out_from_flash = false;
    hwCapabilities->hw_sha = false;  // until will be implemented
    hwCapabilities->hw_int_input = false;
    hwCapabilities->hw_test_mode = false;
    hwCapabilities->hw_freq_set = false;
    hwCapabilities->hw_spi_sdr_min_freq_in_hz = 1000000;
    hwCapabilities->hw_spi_sdr_max_freq_in_hz = FREQ_200_MHZ;
    hwCapabilities->hw_spi_dtr_min_freq_in_hz = 2000000;
    hwCapabilities->hw_spi_dtr_max_freq_in_hz = FREQ_200_MHZ;
    hwCapabilities->hw_spi_max_read_buffer_size = 256;
    hwCapabilities->busModes.plat_spi_1_1_1 = true;
    hwCapabilities->busModes.plat_spi_1_1_2 = true;
    hwCapabilities->busModes.plat_spi_1_2_2 = true;
    hwCapabilities->busModes.plat_spi_1_1_4 = true;
    hwCapabilities->busModes.plat_spi_1_4_4 = true;
    hwCapabilities->busModes.plat_spi_4_4_4 = true;
    hwCapabilities->busModes.plat_spi_1_8_8 = true;
    hwCapabilities->busModes.plat_spi_8_8_8 = true;
    hwCapabilities->hw_test_mode = false;
    hwCapabilities->hw_timer = false; // doesn't support SPI transaction timeout timer
    return 0;
}

int STM32_enable_dqs(bool enable)
{
    if (enable)
        g_dqs_mode = HAL_XSPI_DQS_ENABLE;
    else
        g_dqs_mode = HAL_XSPI_DQS_DISABLE;
    return 0;
}

__inline__ int STM32_writeReadTransaction(PLAT_SPI_FORMAT_T plat_format,
                             uint32_t        flags,
                             const uint8_t*  dataOutStream,
                             uint32_t        cmdSize,
                             uint32_t        addressSize,
                             uint32_t        dataOutSize,
                             uint32_t        dummyCycles,
                             uint8_t*        dataIn,
                             uint32_t        dataInSize)
{


    XSPI_RegularCmdTypeDef  sCommand = {0};
    HAL_StatusTypeDef       retr = HAL_OK;

    sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE; // No alternate bytes phase - TODO: update when required write and read on same transaction

    // Check command
    if (cmdSize == 2)
    {
        sCommand.Instruction = *(uint16_t*)dataOutStream;
        sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    }
    else if (cmdSize == 1)
    {
        sCommand.Instruction = *(uint8_t*)dataOutStream;
        sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    }

    // Check DTR in command
    if (flags)
    {
        sCommand.InstructionDTRMode = (flags & QLIB_SPI_FLAGS__CMD_PHASE_DTR) ?  HAL_XSPI_INSTRUCTION_DTR_ENABLE : 0x00;
        sCommand.AddressDTRMode     = (flags & QLIB_SPI_FLAGS__ADDR_PHASE_DTR) ? HAL_XSPI_ADDRESS_DTR_ENABLE     : 0x00;
        sCommand.DataDTRMode        = (flags & QLIB_SPI_FLAGS__DATA_PHASE_DTR) ? HAL_XSPI_DATA_DTR_ENABLE        : 0x00;
    }

#ifdef STM32N6_XSPI_HIGH_ADDR_WORKAROUND
    // Workaround to overcome STM32N6 HW logic bug to not send address higher than 28bits (which is required for OP1 secure commands)
    // The workaround: Send 'address' as 'dataOut'
    // Note: This workaround doesn't support 1-1-2 & 1-1-4.  since address and data phases have different number of SPI lines.
    if ((addressSize == 4) && (dataInSize == 0))
    {
        // extract address from dataOutStream
        uint32_t address = MAKE32B(dataOutStream[(cmdSize+3)], dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize]);
        if (address > 0x0FFFFFFF)  // check if address is higher than 28bits
        {
            dataOutSize += addressSize;
            addressSize  = 0;
        }
    }
#endif

    // Check command SPI lines
    switch (plat_format)
    {
        case PLAT_SPI_FORMAT_1_1_1:
            sCommand.InstructionMode = cmdSize ? HAL_XSPI_INSTRUCTION_1_LINE : HAL_XSPI_INSTRUCTION_NONE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_1_LINE;
            break;
        case PLAT_SPI_FORMAT_1_1_2:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_2_LINES;
            break;
        case PLAT_SPI_FORMAT_1_2_2:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_2_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_2_LINES;
            break;
        case PLAT_SPI_FORMAT_1_1_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_1_LINE;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
        case PLAT_SPI_FORMAT_1_4_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_4_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
        case PLAT_SPI_FORMAT_4_4_4:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_4_LINES;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_4_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_4_LINES;
            break;
        case PLAT_SPI_FORMAT_1_8_8:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_8_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_8_LINES;
            break;
        case PLAT_SPI_FORMAT_8_8_8:
            sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
            sCommand.AddressMode     = HAL_XSPI_ADDRESS_8_LINES;
            sCommand.DataMode        = HAL_XSPI_DATA_8_LINES;
            break;
        default:
            return -1;
            break;
    }

    // Handle case where there is no CMD
    if (cmdSize == 0)
    {
        sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_NONE;
    }

    // Setup address size
    switch (addressSize)
    {
        case 0:
            sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;
            break;
        case 3:
            sCommand.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
            sCommand.Address            = MAKE32B(dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize], 0);
            break;
        case 4:
            sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
            sCommand.Address            = MAKE32B(dataOutStream[(cmdSize+3)], dataOutStream[(cmdSize+2)], dataOutStream[(cmdSize+1)], dataOutStream[cmdSize]);;
            break;
        default:
            return -1;  // not supported
            break;
    }

    // TODO: handle Alt Bytes (i.e: to support SSE where there can be dataOut and dataIn at the same time)
    if (dataInSize > 0)
    {
        sCommand.DataLength = dataInSize;
    }
    else if (dataOutSize > 0)
    {
        sCommand.DataLength = dataOutSize;
    }
    else
    {
        sCommand.DataLength = 0;
        sCommand.DataMode = HAL_XSPI_DATA_NONE; // no data phase
    }

    // Setup dummy cycles
    sCommand.DummyCycles = dummyCycles;

    // Setup DQS enable/disable (pre-defined)
    sCommand.DQSMode = g_dqs_mode;

    // Process the SPI transaction
    // Send the command (1st part of transaction)
    retr = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    if (retr  != HAL_OK)
    {
        goto error;
    }

    // Read/write data (2nd part of transaction)
    if (dataInSize > 0)  // read data
    {
        retr = HAL_XSPI_Receive(hxspi, dataIn, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
        if (retr != HAL_OK)
        {
            goto error;
        }
    }
    else if (dataOutSize > 0)  // write data
    {
        retr = HAL_XSPI_Transmit(hxspi, dataOutStream + cmdSize + addressSize, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
        if (retr != HAL_OK)
        {
            goto error;
        }
    }

error:
    if (retr != HAL_OK)
    {
        /* Abort any ongoing transaction for the next action */
        (void)HAL_XSPI_Abort(hxspi);
    }

    return 0;
}
