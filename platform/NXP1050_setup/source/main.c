/*---------------------------------------------------------------------------------------------------------*/
/* Winbond Electronics Corporation                                                                         */
/*                                                                                                         */
/*                                                                                                         */
/* Copyright (c) 2019 by Winbond Electronics Corporation                                                   */
/* All rights reserved                                                                                     */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
/* File Content:                                                                                           */
/*            This file serves as main entry point for projects on NXP MXRT10xx                            */
/*            that has a purpose to run and test Atp for Platform related functions				           */
/* Project:                                                                                                */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

 
/**
 * @file    main.c
 * @brief   Application entry point.
 */
//#include <stdio.h>
#include "defs.h"
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MIMXRT1052.h"
#include "fsl_debug_console.h"
#include "comp_test_main.h"
#include "SPI_bridge.h"
#include "SPI_ops.h"

void HW_NXP10xx_ChipInit(void);

/*
 * @brief   Application entry point.
 */
int main(void) {

    /*-----------------------------------------------------------------------------------------------------*/
    /* NXP board initialization                                                                            */
    /*-----------------------------------------------------------------------------------------------------*/
    HW_NXP10xx_ChipInit();
    /*-----------------------------------------------------------------------------------------------------*/
    /* Run ATP tests                                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    CompTestExec();
    /*-----------------------------------------------------------------------------------------------------*/
    /* Recover settings before returning to host                                                           */
    /*-----------------------------------------------------------------------------------------------------*/
    SCB->VTOR = 0x20002000;
    CCM_ANALOG->PLL_ARM = 0x00013042;
    /* Set IPG_PODF. */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 2);

    return 0 ;
}


// NXP10xx HW chip init
void HW_NXP10xx_ChipInit(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    // Init ARM PLL.
    CLOCK_InitArmPll(&armPllConfig_BOARD_BootClockRUN);

    // Set XTAL 24MHz clock frequency.
    CLOCK_SetXtalFreq(24000000U);
    // Set IPG_PODF.
    CLOCK_SetDiv(kCLOCK_IpgDiv, 3);
    BOARD_InitDebugConsole();
    SCB_DisableDCache();

    /*-----------------------------------------------------------------------------------------------------*/
    /* Initialize FlexSPI                                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    flexspi_nor_flash_init(FLEXSPI);
	flexspi_clock_init(FLEXSPI, SPI_FREQUENCY_30MHz, DQS_OFF, DTR_OFF, NULL, false);

}

