#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "common_platform_uart.h"
#include "WTL_Protocol.h"

#ifdef __cplusplus
using namespace COMMON_BOARD_API_UART;
#endif //  __cplusplus

#define PACKET_HEADER_LENGTH 5      // 1 byte sync, 2 bytes for length, 2 bytes for CRC
#define SYNC                 0xe6
#define TIMEOUT_MS           1000

#pragma pack(push, 1)
typedef struct
{
    uint8_t  sync;
    uint16_t length;
    uint16_t crc;
} WTL_PacketHeader;

typedef union
{
    uint8_t    asArray[PACKET_HEADER_LENGTH];
    WTL_PacketHeader asStruct;
} WTL_PacketHeaderUnion;

typedef struct
{
    WTL_PacketHeaderUnion hdr;
    unsigned char* data;
} WTL_Packet;
#pragma pack(pop)

//Functions prototype
static uint16_t calculateCRC_L(const unsigned char* data, uint16_t length);
static void buildPacket_L(WTL_Packet* packet, const unsigned char* buf, const uint16_t size);
static int sendPacket_L(const WTL_Packet* packet);
static int receivePacket_L(unsigned char* buf, uint16_t* size);

static uint16_t calculateCRC_L(const unsigned char* data, uint16_t length)
{
    uint8_t crc = 0;
    uint8_t byte = 0;
    
    for (uint16_t i = 0; i < length; ++i)
    {
        byte = ((uint8_t)data[i]);
        crc ^= byte;
    }
    
    return (uint16_t)crc;
}

static void buildPacket_L(WTL_Packet* packet, const unsigned char* buf, const uint16_t size)
{
    memset(packet, 0x00, sizeof(WTL_Packet));
    packet->hdr.asStruct.sync =   SYNC;
    packet->hdr.asStruct.length = size;
    packet->hdr.asStruct.crc = calculateCRC_L(buf, size);
    packet->data = (unsigned char*)buf;
}

static int sendPacket_L(const WTL_Packet* packet)
{
    int numOfByteTransmitted = 0;
    numOfByteTransmitted += UART_Transmit((unsigned char*)packet->hdr.asArray, PACKET_HEADER_LENGTH);
    numOfByteTransmitted += UART_Transmit((unsigned char*)packet->data, packet->hdr.asStruct.length);
    return numOfByteTransmitted;
}

int receivePacket_L(unsigned char* buf, uint16_t* size)
{
    WTL_PacketHeaderUnion hdr = { 0 };
    uint16_t offset = 0;
    uint16_t totalBytesRead = 0;

    //Wait for SYNC byte (barker)
    do
    {
        UART_ReceiveByte((unsigned char*)&hdr.asStruct.sync);
    } while (hdr.asStruct.sync != SYNC);

    //Read 2 length bytes
    offset = 1; //The offset is 1 after reading the barker
    totalBytesRead = 0;
    do
    {
        totalBytesRead += UART_Receive(&hdr.asArray[offset], sizeof(hdr.asStruct.length) - totalBytesRead);
        offset += totalBytesRead;
    } while (totalBytesRead < sizeof(hdr.asStruct.length));

    //Read 2 CRC bytes
    totalBytesRead = 0;
    do
    {
        totalBytesRead += UART_Receive(&hdr.asArray[offset], sizeof(hdr.asStruct.crc) - totalBytesRead);
        offset += totalBytesRead;
    } while (totalBytesRead < sizeof(hdr.asStruct.crc));

    //Read the data
    totalBytesRead = 0;
    offset = 0;
    do
    {
        totalBytesRead += UART_Receive(&buf[offset], hdr.asStruct.length - totalBytesRead);
        offset += totalBytesRead;
    } while (totalBytesRead < hdr.asStruct.length);

    //Check CRC
    if (calculateCRC_L(&buf[0], hdr.asStruct.length) == hdr.asStruct.crc)
    {
        *size = hdr.asStruct.length;
        return 0;
    }
    return -1;
}

int sendData(const unsigned char* buf, const uint16_t size)
{
    WTL_Packet packet;
    buildPacket_L(&packet, buf, size);
    return sendPacket_L(&packet);
}

int receiveData(unsigned char* buf, uint16_t* size)
{
    return receivePacket_L(buf, size);
}
