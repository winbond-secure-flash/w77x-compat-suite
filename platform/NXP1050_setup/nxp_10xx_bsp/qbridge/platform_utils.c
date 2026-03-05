/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2021 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       platform_utils.c
* @brief      This file includes platform specific implementations for utilities on NXP 10xx device
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
#include "fsl_gpio.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dcp.h"
#include "pin_mux.h"
#include "fsl_trng.h"
#include "fsl_cache.h"
#include "SPI_bridge.h"
#include "common_platform_utils.h"
#include "common_platform_gpio.h"
#if !defined(QLIB_NO_DIRECT_FLASH_ACCESS)
#include "qlib.h"
#endif

#ifndef USER_DEFINED_SHA256
void PLAT_SwSHA256_init(void** ctx);
void PLAT_SwSHA256_update(void* ctx, const uint8_t* data, uint32_t len);
void PLAT_SwSHA256_final(void* ctx, uint8_t* hash);
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                 MACROS                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
#define ROTRIGHT(a,b)       (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z)           (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z)          (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)              (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)              (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)             (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)             (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                  TYPES                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
typedef struct
{
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                GLOBALS                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
static SHA256_CTX hashCtx_L;
#endif
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                CONSTANTS                                                */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
static const uint32_t k[64] =
{
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       LOCAL FUNCTION DECLARATION                                        */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]);
void PLAT_SwSHA256_init(void** ctx);
void PLAT_SwSHA256_update(void* ctx, const uint8_t* data, uint32_t len);
void PLAT_SwSHA256_final(void* ctx, uint8_t* hash);
#endif
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_Init(void* user_data, uint32_t spiFreq)
{
    //Waiting 5 seconds for all components to be ready
    PLAT_DelayMsec(5000);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_ConfigMPU();

    /*-----------------------------------------------------------------------------------------------------*/
    /* Init SPI bus                                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    flexspi_nor_flash_init(FLEXSPI);
    PLAT_SPI_SetFreqInHz(spiFreq);
    PLAT_SPI_EnableDQS(false);

#ifdef SPI_LUT_OPTIMIZATION_ENABLED
#ifdef QLIB_NO_DIRECT_FLASH_ACCESS // Can't replace MCU fetch command when direct access is enabled
    flexspi_secure_LUT_init(QLIB_BUS_MODE_1_1_1, false); // Initialize for single SPI. User should then update for any interface needed
#endif
#endif

    SCB_DisableDCache();
    PRINTF("D-Cache Disabled\n\r");

#ifdef USER_DEFINED_SHA256
    /*-----------------------------------------------------------------------------------------------------*/
    /* Init dcp (SHA-256) module                                                                           */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        dcp_config_t dcpConfig;

        DCP_GetDefaultConfig(&dcpConfig);
        DCP_Init(DCP, &dcpConfig);
    }
#endif // USER_DEFINED_SHA256

#ifdef USER_DEFINED_NONCE
    /*-----------------------------------------------------------------------------------------------------*/
    /* Init TRNG module                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        trng_config_t userConfig;

        TRNG_GetDefaultConfig(&userConfig);
        TRNG_Init(TRNG, &userConfig);
    }
#endif // USER_DEFINED_NONCE
    return 0;
}

void PLAT_waitForSwitch(void)
{
    // Define the init structure for the input switch pin
    gpio_pin_config_t sw_config = {
            kGPIO_DigitalInput,
            0,
            kGPIO_NoIntmode,
    };

    // Init input switch GPIO.
    GPIO_PinInit(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN, &sw_config);

    // Wait till SW8 is pressed
    PRINTF("Execution is halt press SW8 to continue \r\n");
    while (1 == GPIO_PinRead(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN));
    PRINTF("Continue the execution\r\n");
}

uint32_t PLAT_GetCpuFreq(void)
{
    return SystemCoreClock;
}

int PLAT_FlashType(bool* val)
{
    *val = true;
    return 0;
}

__attribute__((optimize("O0")))
int PLAT_DelayMsec(uint32_t mSec)
{
    if (mSec == 0)
    {
        return 0;
    }

#ifdef DELAY_TIMER_ID
     return PLAT_TIMER_Delay(DELAY_TIMER_ID, mSec);
#else
    uint64_t count =  ((uint64_t)mSec) * (((uint64_t)PLAT_GetCpuFreq()) / (4 * 1000U) );

    // 4*count clocks delay loop
    __ASM volatile("    MOV    R0, %0" : : "r"((uint32_t)count));
    __ASM volatile(
        "loop:                          \n"
        "    SUB    R0, R0, #1          \n"
        "    CMP    R0, #0              \n"
        "    BNE    loop                \n");

    return 0;
#endif
}

#ifndef USER_DEFINED_SHA256
int PLAT_SwSHA256WithPadding(uint32_t* output, const void* data, uint32_t dataSize, uint32_t padValue, uint32_t padSize)
{
    uint32_t   i = 0;
    void* ctx;

    PLAT_SwSHA256_init(&ctx);
    PLAT_SwSHA256_update(ctx, (const uint8_t*)data, dataSize);
    for (i = 0; i < padSize / sizeof(uint32_t); ++i)
    {
        PLAT_SwSHA256_update(ctx, (const uint8_t*)(&padValue), sizeof(uint32_t));
    }
    PLAT_SwSHA256_final(ctx, (uint8_t*)output);
    return 0;
}
#endif

int PLAT_SHA256WithPadding(uint32_t* output, const void* data, uint32_t dataSize, uint32_t padValue, uint32_t padSize)
{
#ifdef USER_DEFINED_SHA256
    dcp_handle_t m_handle;
    size_t outLength = 256 >> 3; // 256 bit
    dcp_hash_ctx_t hashCtx;
    uint32_t i = 0;

    m_handle.channel    = kDCP_Channel0;
    m_handle.keySlot    = kDCP_KeySlot0;
    m_handle.swapConfig = kDCP_NoSwap;

    if (DCP_HASH_Init(DCP, &m_handle, &hashCtx, kDCP_Sha256) != kStatus_Success)
    {
        return -1;
    }

    if (DCP_HASH_Update(DCP, &hashCtx, (const uint8_t*)data, dataSize) != kStatus_Success)
    {
        return -1;
    }

    for (i = 0; i < padSize / sizeof(uint32_t); ++i)
    {
        if (DCP_HASH_Update(DCP, &hashCtx, (const uint8_t*)(&padValue), sizeof(uint32_t)) != kStatus_Success)
        {
            return -1;
        }
    }

    if (DCP_HASH_Finish(DCP, &hashCtx, (uint8_t*)output, &outLength) != kStatus_Success)
    {
        return -1;
    }
    return 0;
#else
    STATUS_RET_CHECK(PLAT_SwSHA256WithPadding(output, data, dataSize, padValue, padSize));
    return 0;
#endif
}

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                             LOCAL FUNCTIONS                                             */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef USER_DEFINED_SHA256
void sha256_transform(SHA256_CTX *ctx, const uint8_t data[])
{
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];    ///<NOLINT

    for (i = 0, j = 0; i < 16; ++i, j += 4)
    {
        m[i] =  (((uint32_t)data[j])     << 24)  |
                (((uint32_t)data[j + 1]) << 16)  |
                ((uint32_t)(data[j + 2]) << 8)   |
                ((uint32_t)(data[j + 3]));
    }

    for ( ; i < 64; ++i)
    {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i)
    {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void PLAT_SwSHA256_init(void** ctx)
{
    hashCtx_L.datalen = 0;
    hashCtx_L.bitlen = 0;
    hashCtx_L.state[0] = 0x6a09e667;
    hashCtx_L.state[1] = 0xbb67ae85;
    hashCtx_L.state[2] = 0x3c6ef372;
    hashCtx_L.state[3] = 0xa54ff53a;
    hashCtx_L.state[4] = 0x510e527f;
    hashCtx_L.state[5] = 0x9b05688c;
    hashCtx_L.state[6] = 0x1f83d9ab;
    hashCtx_L.state[7] = 0x5be0cd19;
    *ctx = &hashCtx_L;
}

void PLAT_SwSHA256_update(void* ctx, const uint8_t* data, uint32_t len)
{
    uint32_t i;
    SHA256_CTX* shaCtx = (SHA256_CTX*)(ctx);

    for (i = 0; i < len; ++i)
    {
        shaCtx->data[shaCtx->datalen] = data[i];
        shaCtx->datalen++;
        if (shaCtx->datalen == 64)
        {
            sha256_transform(shaCtx, shaCtx->data);
            shaCtx->bitlen += 512;
            shaCtx->datalen = 0;
        }
    }
}

void PLAT_SwSHA256_final(void* ctx, uint8_t* hash)
{
    uint32_t i;
    SHA256_CTX* shaCtx = (SHA256_CTX*)(ctx);

    i = shaCtx->datalen;

    // Pad whatever data is left in the buffer.
    if (shaCtx->datalen < 56)
    {
        shaCtx->data[i++] = 0x80;
        while (i < 56)
        {
            shaCtx->data[i++] = 0x00;
        }
    }
    else
    {
        shaCtx->data[i++] = 0x80;
        while (i < 64)
        {
            shaCtx->data[i++] = 0x00;
        }
        sha256_transform(shaCtx, shaCtx->data);
        memset(shaCtx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    shaCtx->bitlen += shaCtx->datalen * 8;
    shaCtx->data[63] = (uint8_t)(shaCtx->bitlen & 0xFF);
    shaCtx->data[62] = (uint8_t)((shaCtx->bitlen >> 8)  & 0xFF);
    shaCtx->data[61] = (uint8_t)((shaCtx->bitlen >> 16) & 0xFF);
    shaCtx->data[60] = (uint8_t)((shaCtx->bitlen >> 24) & 0xFF);
    shaCtx->data[59] = (uint8_t)((shaCtx->bitlen >> 32) & 0xFF);
    shaCtx->data[58] = (uint8_t)((shaCtx->bitlen >> 40) & 0xFF);
    shaCtx->data[57] = (uint8_t)((shaCtx->bitlen >> 48) & 0xFF);
    shaCtx->data[56] = (uint8_t)((shaCtx->bitlen >> 56) & 0xFF);
    sha256_transform(shaCtx, shaCtx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i)
    {
        hash[i]      = (shaCtx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (shaCtx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (shaCtx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (shaCtx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (shaCtx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (shaCtx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (shaCtx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (shaCtx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}


#endif
