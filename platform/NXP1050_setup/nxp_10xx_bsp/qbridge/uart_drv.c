/*---------------------------------------------------------------------------------------------------------*/
/*  Winbond Technology Corporation                                                                         */
/*                                                                                                         */
/*  Copyright (c) 2020 by Winbond Technology Corporation                                                   */
/*  All rights reserved                                                                                    */
/*                                                                                                         */
/*<<<------------------------------------------------------------------------------------------------------*/
/* File Contents:                                                                                          */
/*   uart_drv.c                                                                                            */
/*            NXP UART implementation                                                                      */
/* Project:                                                                                                */
/*            All Windows projects                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include "board.h"
#include "fsl_debug_console.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 EXTERNS                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
extern int DbgConsole_SendDataReliable(uint8_t *ch, size_t size);
extern int DbgConsole_ReadCharacter(uint8_t *ch);
int DbgConsole_ReadLine(uint8_t *buf, size_t size);

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                              IMPLEMENTATION                                             */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

// Open COM port
// Returns -1 in case of an error. return 0 is OK
// In case the comport is already opened (by another process), it will not open the port and will return an error instead.
// baudrate range [300bps - 3000000bps]
int UART_Init(int com, int baudrate)
{
    uint32_t uartClkSrcFreq = BOARD_DebugConsoleSrcFreq();
    return DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, baudrate, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

// Close COM port
int UART_Finit(void)
{
    return DbgConsole_Deinit();
}

// UART Tx function
// Returns - 1 in case of an error, otherwise it returns the amount of bytes sent.
// This function is blocking (it returns after all the bytes have been processed).
int UART_Transmit(unsigned char* buf, unsigned int size)
{
    return DbgConsole_SendDataReliable(buf, size);
}

// UART non-blocking 1 Byte Rx function
// return 1 if one byte received (if no data received, return 0)
int UART_ReceiveByte(unsigned char* ch)
{
    return DbgConsole_ReadCharacter(ch);
}

// UART non-blocking Rx function
// return num of bytes received (if no data received, return 0)
int UART_Receive(unsigned char* buf, unsigned int size)
{
    uint32_t i = 0;
    unsigned char ch;

    for (i = 0; i < size; ++i)
    {
        if(1 != DbgConsole_ReadCharacter(&ch))
        {
            return -1;
        }
        else
        {
            buf[i] = ch;
        }
    }
    return size;
}

// UART blocking Rx function until timeout [msec]
// return num of bytes received (if no data received, return 0)
int UART_ReceiveTimeout(unsigned char* buf, unsigned int size, unsigned int timeoutMs)
{
    return 0;
}

// Discards all characters from the output or input buffer
int UART_Flush(void)
{
    return 0;
}
