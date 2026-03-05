#ifndef WTL_PROTOCOL_H_
#define WTL_PROTOCOL_H_

#include <stdint.h>
#include "common_platform_uart.h" 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int sendData(const unsigned char* buf, const uint16_t size);
int receiveData(unsigned char* buf, uint16_t* size);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif //__cplusplus

#endif // WTL_PROTOCOL_H_
