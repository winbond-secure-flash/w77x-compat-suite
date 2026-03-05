/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2020 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       common_platform_uart.h
* @brief      This file includes Windows UART interface
*
* ### project W77Q
*
************************************************************************************************************/

#ifndef COMMON_PLATFORM_UART_H__
#define COMMON_PLATFORM_UART_H__

#include <stdint.h>

#ifdef __cplusplus
namespace COMMON_BOARD_API_UART
{
#endif //  __cplusplus

#define UART_115200_BPS         (115200)
#define UART_230400_BPS         (230400)

    // Open COM port
    // Returns -1 in case of an error.
    // In case the comport is already opened (by another process), it will not open the port and will return an error instead.
    // baudrate range [300bps - 3000000bps]
    int UART_Init(int com, int baudrate);

    // Close COM port
    int UART_Finit(void);

    // UART Tx function
    // Returns -1 in case of an error, otherwise it returns the amount of bytes sent.
    // This function is blocking (it returns after all the bytes have been processed).
    int UART_Transmit(unsigned char* buf, unsigned int size);

    // UART non-blocking 1 Byte Rx function
    // return 1 if one byte received (if no data received, return 0)
    int UART_ReceiveByte(unsigned char* ch);

    // UART non-blocking Rx function
    // return num of bytes received (if no data received, return 0)
    int UART_Receive(unsigned char* buf, unsigned int size);

    // UART blocking Rx function until timeout [msec]
    // return num of bytes received (if no data received, return 0)
    int UART_ReceiveTimeout(unsigned char* buf, unsigned int size, unsigned int timeoutMs);

    // Discards all characters from the output or input buffer
    int UART_Flush(void);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif // COMMON_PLATFORM_UART_H__
