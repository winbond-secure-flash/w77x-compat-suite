/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2019 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       qlib_platform.c
* @brief      This file contains platform specific implementations on NXP 10xx device
*
* ### project qlib
*
************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                INCLUDES                                                 */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#include "SPI_ops.h"
#include "SPI_bridge.h"
#include "fsl_trng.h"
#include "fsl_dcp.h"
#include "fsl_gpio.h"
#include "common_platform_sha256.h"
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                               DEFINITIONS                                               */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

//Replace the platform SHA with SHA256 algorithm
#define QLIB_PLAT_SHA256_Init   PLAT_HASH_Init
#define QLIB_PLAT_SHA256_Update PLAT_HASH_Update
#define QLIB_PLAT_SHA256_Finish PLAT_HASH_Finish
#ifdef Q2_API
#define PLAT_SHA256        PLAT_HASH
#ifdef QLIB_HASH_OPTIMIZATION_ENABLED
#define PLAT_SHA256_Async PLAT_HASH_Async
#define PLAT_SHA256_Async_WaitWhileBusy PLAT_HASH_Async_WaitWhileBusy
#endif
#endif

#define BLOCK_SIZE 64u
#define HASH_SIZE  32u
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                GLOBALS                                                  */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
extern SPI_FREQUENCY_T g_set_spi_freq;
extern bool            g_set_dqs_mode;

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       LOCAL FUNCTION DECLARATION                                        */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
static void H(const void* x, const uint32_t xLen, const void* y, const uint32_t yLen, void* out);
int PLAT_SHA256_InitOpt(void** ctx, bool opt55);

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       PLATFORM SPECIFIC FUNCTIONS                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

#define FLASH_HW_RESET_GPIO_PORT    GPIO1
#define FLASH_HW_RESET_GPIO_PIN     (14U)

__attribute__ ((section(".ramfunc.$RAM")))
void FlashReset(void)
{
    volatile uint32_t i;
	gpio_pin_config_t flashHwResetGPIO_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};
	GPIO_PinInit(FLASH_HW_RESET_GPIO_PORT, FLASH_HW_RESET_GPIO_PIN, &flashHwResetGPIO_config);
    GPIO_PinWrite(FLASH_HW_RESET_GPIO_PORT, FLASH_HW_RESET_GPIO_PIN, 0);
    for(i=0;i<100000;i++);
    GPIO_PinWrite(FLASH_HW_RESET_GPIO_PORT, FLASH_HW_RESET_GPIO_PIN, 1);
    for(i=0;i<100000;i++); // recovery time
}


__attribute__ ((section(".ramfunc.$RAM")))
void CORE_RESET(void)
{
	FlashReset();
	__NVIC_SystemReset();
}

int QLIB_PLAT_SHA256_Init(void** ctx, QLIB_HASH_OPT_T opt)
{
    return PLAT_SHA256_InitOpt(ctx, opt == QLIB_HASH_OPT_FIXED_55_ALIGNED ? true : false);
}

int QLIB_PLAT_SHA256_Update(void* ctx, const void* data, uint32_t dataSize)
{
	return PLAT_SHA256_Update(ctx, data, dataSize);
}

int QLIB_PLAT_SHA256_Finish(void* ctx, uint32_t* output)
{
	return PLAT_SHA256_Finish(ctx, output);
}

#define PLAT_ADDR_ALIGNED32(addr) (0u == (((uint32_t)addr) & 0x3u))

#ifdef Q2_API
void PLAT_SHA256(uint32_t* output, const void* data, uint32_t dataSize)
{
	void*         ctx;
	PLAT_SHA256_InitOpt(&ctx, (dataSize == 55u) && PLAT_ADDR_ALIGNED32(data) ? true : false);
	PLAT_SHA256_Update(ctx, data, dataSize);
	PLAT_SHA256_Finish(ctx, output);
}

#ifdef QLIB_HASH_OPTIMIZATION_ENABLED
static void* PLAT_SHA256_Async_ctx_l = NULL;
static uint32_t* PLAT_SHA256_Async_Output_l = NULL;

void PLAT_SHA256_Async(uint32_t* output, const void* data, uint32_t dataSize)
{
    PLAT_SHA256_InitOpt(&PLAT_SHA256_Async_ctx_l, true);
    PLAT_SHA256_Update(PLAT_SHA256_Async_ctx_l, data, dataSize);
    PLAT_SHA256_Async_Output_l = output;
}

void PLAT_SHA256_Async_WaitWhileBusy(void)
{
    PLAT_SHA256_Finish(PLAT_SHA256_Async_ctx_l, PLAT_SHA256_Async_Output_l);
    PLAT_SHA256_Async_ctx_l = NULL;
    PLAT_SHA256_Async_ctx_l = NULL;
}

#endif // #ifdef QLIB_HASH_OPTIMIZATION_ENABLED
#endif // Q2_API

uint64_t PLAT_GetNONCE(void)
{
#ifdef USER_DEFINED_NONCE
    uint64_t nonce;
    TRNG_GetRandomData(TRNG, &nonce, sizeof(nonce));

    return nonce;
#else
    /*-----------------------------------------------------------------------------------------------------*/
    /* We use here 64bit XORSHIFT algorithm for PRNG                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    static uint64_t nonce = 0x500DF00D;

    nonce ^= nonce << 13;
    nonce ^= nonce >> 7;
    nonce ^= nonce << 17;

    return nonce;
#endif
}

__attribute__ ((section(".ramfunc.$RAM")))
int PLAT_SPI_WriteReadTransaction(const void*     userData,
                                  QLIB_BUS_MODE_T format,
                                  uint32_t        flags,
                                  const uint8_t*  dataOutStream,
                                  uint32_t        cmdSize,
                                  uint32_t        addressSize,
                                  uint32_t        dataOutSize,
                                  uint32_t        dummyCycles,
                                  uint8_t*        dataIn,
                                  uint32_t        dataInSize)
{
    return NXP10xx_writeReadTransaction((PLAT_SPI_FORMAT_T)format, flags, dataOutStream, cmdSize, addressSize,
            dataOutSize, dummyCycles, dataIn, dataInSize);
}

// SW PLAT_HMAC implementation
void PLAT_HMAC(uint8_t* output, const uint8_t* key, uint32_t keySize, const void* data, uint32_t dataSize)
{
	// SW implementation of HMAC function
    uint8_t  k[BLOCK_SIZE];
    uint8_t  k_ipad[BLOCK_SIZE];
    uint8_t  k_opad[BLOCK_SIZE];
    uint8_t  iHash[HASH_SIZE];
    uint8_t  oHash[HASH_SIZE];
    uint32_t i;

    (void)memset(k, 0, sizeof(k));
    (void)memset(k_ipad, 0x36, BLOCK_SIZE);
    (void)memset(k_opad, 0x5c, BLOCK_SIZE);

    if (keySize > BLOCK_SIZE)
    {
        // If the key is larger than the hash algorithm's
        // block size, we must digest it first.
        void*    hashCtx;
        uint32_t digest[8];
        PLAT_HASH_Init(&hashCtx, QLIB_HASH_OPT_NONE);
        PLAT_HASH_Update(hashCtx, key, keySize);
        PLAT_HASH_Finish(hashCtx, digest);
        (void)memcpy(k, digest, sizeof(digest));
    }
    else
    {
        (void)memcpy(k, key, keySize);
    }

    for (i = 0; i < BLOCK_SIZE; i++)
    {
        k_ipad[i] ^= k[i];
        k_opad[i] ^= k[i];
    }

    // Perform HMAC algorithm:
    // H(K XOR opad, H(K XOR ipad, data))
    H(k_ipad, sizeof(k_ipad), data, dataSize, iHash);
    H(k_opad, sizeof(k_opad), iHash, sizeof(iHash), oHash);

    (void)memcpy(output, oHash, HASH_SIZE);
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                             LOCAL FUNCTIONS                                             */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

// Concatenate X & Y, return hash.
static void H(const void* x, const uint32_t xLen, const void* y, const uint32_t yLen, void* out)
{
    void*    hashCtx;
    uint32_t hashOutput[8];

    PLAT_HASH_Init(&hashCtx, QLIB_HASH_OPT_NONE);

    PLAT_HASH_Update(hashCtx, x, xLen);
    PLAT_HASH_Update(hashCtx, y, yLen);
    PLAT_HASH_Finish(hashCtx, hashOutput);
    (void)memcpy(out, hashOutput, sizeof(hashOutput));
}
