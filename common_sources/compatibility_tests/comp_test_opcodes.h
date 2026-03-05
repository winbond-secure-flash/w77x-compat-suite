/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2025 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_opcodes.h
* @brief      This file includes definitions of SPI flash opcodes
*
*
************************************************************************************************************/
#ifndef __COMP_TEST_OPCODES_H__
#define __COMP_TEST_OPCODES_H__

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 DEFINES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/*                                    FLASH COMMANDS                                                       */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Standard SPI Instructions                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__NONE 0x00u

#define SPI_FLASH_CMD__READ_JEDEC                      0x9F
#define SPI_FLASH_CMD__DEVICE_ID                       0xAB
#define SPI_FLASH_CMD__MANUFACTURER_AND_DEVICE_ID      0x90u
#define SPI_FLASH_CMD__MANUFACTURER_AND_DEVICE_ID_DUAL 0x92
#define SPI_FLASH_CMD__MANUFACTURER_AND_DEVICE_ID_QUAD 0x94

/*---------------------------------------------------------------------------------------------------------*/
/* Write Instructions                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__WRITE_ENABLE  0x06u
#define SPI_FLASH_CMD__WRITE_DISABLE 0x04u

#define SPI_FLASH_CMD__PAGE_PROGRAM       0x02u
#define SPI_FLASH_CMD__PAGE_PROGRAM_1_4_4 0x32u

/*---------------------------------------------------------------------------------------------------------*/
/* Erase Instructions                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__ERASE_SECTOR          0x20u
#define SPI_FLASH_CMD__ERASE_BLOCK_32        0x52u
#define SPI_FLASH_CMD__ERASE_BLOCK_64        0xD8u
#define SPI_FLASH_CMD__ERASE_CHIP            0xC7u
#define SPI_FLASH_CMD__ERASE_CHIP_DEPRECATED 0x60u

/*---------------------------------------------------------------------------------------------------------*/
/* SOI/QPI/OPI mode                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__ENTER_SPI 0xFFu // This is EXIT_QPI command in W77Q (16Mb to 128Mb) chips
#define SPI_FLASH_CMD__ENTER_QPI 0x38u
#define SPI_FLASH_CMD__ENTER_OPI 0xE8u

#define SPI_FLASH_CMD__ENTER_SOPI_MODE 0x81u
#define SPI_FLASH_CMD__ENTER_DOPI_MODE 0x82u
#define SPI_FLASH_CMD__RESET_SPI_MODE  0xFFu

/*---------------------------------------------------------------------------------------------------------*/
/* Status Registers                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__READ_STATUS_REGISTER_1  0x05
#define SPI_FLASH_CMD__READ_STATUS_REGISTER_2  0x35
#define SPI_FLASH_CMD__WRITE_STATUS_REGISTER_2 0x31
#define SPI_FLASH_CMD__READ_STATUS_REGISTER_3  0x15


/************************************************************************************************************
 * Read Instructions
************************************************************************************************************/
// *** The Quad Enable (QE) bit in Status Register-2 / CR must be set to 1 before the device will accept the Fast Read Quad/Octal Output Instruction
#define SPI_FLASH_CMD__READ_DATA__1_1_1     0x03
#define SPI_FLASH_CMD__READ_FAST__1_1_1     0x0B
#define SPI_FLASH_CMD__READ_FAST__1_1_2     0x3B
#define SPI_FLASH_CMD__READ_FAST__1_2_2     0xBB
#define SPI_FLASH_CMD__READ_FAST__1_1_4     0x6B
#define SPI_FLASH_CMD__READ_FAST__1_4_4     0xEB
#define SPI_FLASH_CMD__READ_FAST__1_8_8     0xCB
#define SPI_FLASH_CMD__READ_FAST__4_4_4     0xEB
#define SPI_FLASH_CMD__READ_FAST__8_8_8     0xCB
#define SPI_FLASH_CMD__READ_FAST_DTR__1_1_1 0x0D
#define SPI_FLASH_CMD__READ_FAST_DTR__1_2_2 0xBD
#define SPI_FLASH_CMD__READ_FAST_DTR__1_4_4 0xED
#define SPI_FLASH_CMD__READ_FAST_DTR__1_8_8 0xCD
#define SPI_FLASH_CMD__READ_FAST_DTR__4_4_4 0xED
#define SPI_FLASH_CMD__READ_FAST_DTR__8_8_8 0xCD

/*---------------------------------------------------------------------------------------------------------*/
/* Extended Configuration Register Instructions                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__READ_CR       0x85
#define SPI_FLASH_CMD__WRITE_CR      0x81
#define SPI_FLASH_CMD__READ_CR_DFLT  0xB5
#define SPI_FLASH_CMD__WRITE_CR_DFLT 0xB1
#define SPI_FLASH_DUMMY_CYCLES__CR   (8)

/*---------------------------------------------------------------------------------------------------------*/
/* Reset Instructions                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__RESET_ENABLE 0x66
#define SPI_FLASH_CMD__RESET_DEVICE 0x99

/*---------------------------------------------------------------------------------------------------------*/
/* Address mode Instructions                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define SPI_FLASH_CMD__4_BYTE_ADDRESS_MODE_ENTER 0xB7u
#define SPI_FLASH_CMD__4_BYTE_ADDRESS_MODE_EXIT  0xE9u


/*---------------------------------------------------------------------------------------------------------*/
/* Secure Commands                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define GET_ACLR_OPCODE_CTAG (0x3C000000)
#define SET_ACLR_OPCODE_CTAG (0x2C000000)

#define SPI_FLASH_CMD__OP0_1_1_1       0xA0
#define SPI_FLASH_CMD__OP1_1_1_1       0xA1
#define SPI_FLASH_CMD__OP2_1_1_1       0xA2
#define SPI_SSR_RESPONSE_COUNT         4

/*---------------------------------------------------------------------------------------------------------*/
/* CR register offset                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define CR_ADDR_SPI_BUS_MODES 0x00
#define CR_ADDR_DUMMY_CYCLES  0x01

#define CR_DUMMY_FIELD  0u, 5u
#define CR_DQS_EN_FIELD 5u, 1u
#define SR2_QE_FIELD    1u, 1u


#endif // __COMP_TEST_OPCODES_H__
