/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2024 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_capabilities.h
* @brief      This file includes platform capabilities
*
* ### project W77Q
*
************************************************************************************************************/
#ifndef COMMON_PLATFORM_CAPABILITIES_H__
#define COMMON_PLATFORM_CAPABILITIES_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "common_platform_spi.h"

typedef struct PLAT_SPI_HW_CAPABILITIES_T
{
    PLAT_SPI_BUSMODE_SUPPORTED_T busModes;
    bool dtr;
    bool dqs;
    bool hw_gpio;
    bool hw_flash_voltage_set;
    bool hw_reset_to_flash;
    bool reset_out_from_flash;
    bool hw_leds;
    bool hw_timer;
    bool hw_sha;
    bool hw_int_input;
    bool hw_test_mode;
    bool hw_freq_set;
    uint32_t hw_spi_sdr_min_freq_in_hz;
    uint32_t hw_spi_sdr_max_freq_in_hz;
    uint32_t hw_spi_dtr_min_freq_in_hz;
    uint32_t hw_spi_dtr_max_freq_in_hz;
    uint32_t hw_spi_max_read_buffer_size;
    bool hw_socket_type_identify;
    uint8_t reserved[19];
} PLAT_SPI_HW_CAPABILITIES_T;

int PLAT_GetCapabilities(PLAT_SPI_HW_CAPABILITIES_T* hwCapabilities);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif  // COMMON_PLATFORM_CAPABILITIES_H__
