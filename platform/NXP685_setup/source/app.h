/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*${header:start}*/
#include "fsl_cache.h"
/*${header:end}*/
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI                 FLEXSPI
#define EXAMPLE_CACHE                   CACHE64
#define FLASH_SIZE                      0x8000 /* 256Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI_AMBA_BASE
//#ifndef FLASH_PAGE_SIZE
//#define FLASH_PAGE_SIZE                 256
//#endif
#define EXAMPLE_SECTOR                  1000
#define SECTOR_SIZE                     0x1000 /* 4K */
#define EXAMPLE_FLEXSPI_CLOCK           kCLOCK_Flexspi
//#define FLASH_PORT                      kFLEXSPI_PortB1
#define FLASH_PORT                      kFLEXSPI_PortA1
//#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkExternalInputFromDqsPad
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackInternally
//#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackFromSckPad

#if 0  // NXP original LUT indexes
#define NOR_CMD_LUT_SEQ_IDX_READ            0
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS      1
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE     2
#define NOR_CMD_LUT_SEQ_IDX_READID_OPI      3
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_OPI 4
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR     5
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE       6
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM     7
#define NOR_CMD_LUT_SEQ_IDX_ENTEROPI        8
/* NOTE: Workaround for debugger.
   Must define AHB write FlexSPI sequence index to 9 to avoid debugger issue.
   Debugger can attach to the CM33 core only when ROM executes to certain place.
   At that point, AHB write FlexSPI sequence index is set to 9, but in LUT, the
   command is not filled by ROM. If the debugger sets software breakpoint at flash
   after reset/attachment, FlexSPI AHB write command will be triggered. It may
   cause AHB bus hang if the command in LUT sequence index 9 is any read opeartion.
   So we need to ensure at any time, the FlexSPI LUT sequence 9 for the flash must
   be set to STOP command to avoid unexpected debugger behaivor.
 */
#define NOR_CMD_LUT_SEQ_IDX_WRITE          9
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_OPI 10
#endif

// Winbond LUT index
#define NOR_CMD_LUT_SEQ_IDX_READ_SR1_SPI                 (0)
#define NOR_CMD_LUT_SEQ_IDX_READ_SR1_DOPI                (1)
#define NOR_CMD_LUT_SEQ_IDX_READ_CR_3B_ADDR_DUMMY_SPI    (2)
#define NOR_CMD_LUT_SEQ_IDX_WRITE_CR_3B_ADDR_DUMMY_SPI   (3)
#define NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE_SPI             (4)
#define NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE_DOPI            (5)
#define NOR_CMD_LUT_SEQ_IDX_READ_JEDEC_ID                (6)
#define NOR_CMD_LUT_SEQ_IDX_READ_JEDEC_ID_DOPI           (7)
#define NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR_3B_ADDR_SPI     (8)
#define NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR_4B_ADDR_DOPI    (9)
#define BAD_CMD_LUT_SEQ_IDX                             (10) // for some reason index 10 doesn't output signals
#define NOR_CMD_LUT_SEQ_IDX_ENTER_DOPI                  (11)
#define NOR_CMD_LUT_SEQ_IDX_ENTER_SOPI                  (12)
#define NOR_CMD_LUT_SEQ_IDX_READ_JEDEC_ID_SOPI          (13)
#define NOR_CMD_LUT_SEQ_IDX_ENTER_QPI_TEMP              (14)
#define NOR_CMD_LUT_SEQ_IDX_ENTER_QPI                   (16)
#define NOR_CMD_LUT_SEQ_IDX_READ_JEDEC_ID_QPI_TEMP      (15)
#define NOR_CMD_LUT_SEQ_IDX_READ_JEDEC_ID_QPI           (18)
#define NOR_CMD_LUT_SEQ_IDX_ENTER_SPI_FROM_QPI          (17)
#define NOR_CMD_LUT_SEQ_IDX_WRITE_NOP_1_8_8             (19)
#define NOR_CMD_LUT_SEQ_IDX_READ_NOP_1_8_8              (20)
#define NOR_CMD_LUT_SEQ_IDX_WRITE_NOP_1_1_1             (21)
#define NOR_CMD_LUT_SEQ_IDX_READ_NOP_1_1_1              (22)


#define CUSTOM_LUT_LENGTH        124
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
#define FLASH_ERROR_STATUS_MASK  0x0e
#define CACHE_MAINTAIN           0
//#define EXAMPLE_FLASH_RESET_CONFIG()           \
//    do                                         \
//    {                                          \
//        GPIO->B[2][12] = 0;                    \
//        for (uint32_t i = 0; i < 10000UL; i++) \
//        {                                      \
//            __NOP();                           \
//        }                                      \
//        GPIO->B[2][12] = 1;                    \
//    } while (0)

/*${macro:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
typedef struct _flexspi_cache_status
{
    volatile bool CacheEnableFlag;
} flexspi_cache_status_t;
/*${variable:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
