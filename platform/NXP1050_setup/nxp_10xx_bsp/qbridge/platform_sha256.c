/************************************************************************************************************
* \internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2024 by Winbond Electronics Corporation . All rights reserved
* \endinternal
*
* @file       platform_sha256.c
* @brief      This file contains platform specific implementations on NXP 10xx device
*
* ### project qlib
*
************************************************************************************************************/
#include "common_platform_sha256.h"
#include "fsl_dcp.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef QLIB_PERF
#include "qlib_platform.h"
#endif

#ifndef USER_DEFINED_SHA256
extern void PLAT_SwSHA256_init(void** ctx);
extern void PLAT_SwSHA256_update(void* ctx, const uint8_t* data, uint32_t len);
extern void PLAT_SwSHA256_final(void* ctx, uint8_t* hash);
#endif

#define HASH_SIZE  32u

#ifdef USER_DEFINED_SHA256

struct DCP_HASH_CTX_T
{
	dcp_hash_ctx_t ctx;
	dcp_handle_t   handle;
};

struct OPT_55_HASH_CTX_T
{
    volatile uint32_t runningHash[9];
    dcp_work_packet_t dcpWork;
};

typedef struct PLAT_HASH_CTX_T {
  bool typeOpt55;
  union {
	  struct DCP_HASH_CTX_T hashCtx;
	  struct OPT_55_HASH_CTX_T opt55hashCtx;
  } data;
} PLAT_HASH_CTX_T;

static struct PLAT_HASH_CTX_T hashCtx_L;
#endif

enum _dcp_work_packet_bit_definitions
{
    kDCP_CONTROL0_DECR_SEMAPHOR      = 1u << 1,  /* DECR_SEMAPHOR */
    kDCP_CONTROL0_ENABLE_HASH        = 1u << 6,  /* ENABLE_HASH */
    kDCP_CONTROL0_HASH_INIT          = 1u << 12, /* HASH_INIT */
    kDCP_CONTROL0_HASH_TERM          = 1u << 13, /* HASH_TERM */
    kDCP_CONTROL1_HASH_SELECT_SHA256 = 2u << 16,
    kDCP_CONTROL1_HASH_SELECT_SHA1   = 0u << 16,
    kDCP_CONTROL1_HASH_SELECT_CRC32  = 1u << 16,
};

/************************************************************************************************************
 * @brief       Finalize platform
 *
 * @param[in]  opt55        following data is 55 bytes in aligned address
 *
 * @return      None
************************************************************************************************************/
int PLAT_SHA256_InitOpt(void** ctx, bool opt55)
{
#ifdef USER_DEFINED_SHA256
    status_t status;

    hashCtx_L.data.hashCtx.handle.channel    = kDCP_Channel0;
    hashCtx_L.data.hashCtx.handle.keySlot    = kDCP_KeySlot0;
    hashCtx_L.data.hashCtx.handle.swapConfig = kDCP_NoSwap;

#if (defined USE_BL_HOST) || (defined STRESS_FROM_HOST) || (defined BIN_LOADER)
     status = DCP_HASH_Init(DCP, &hashCtx_L.data.hashCtx.handle, &(hashCtx_L.data.hashCtx.ctx), kDCP_Sha256);
     hashCtx_L.typeOpt55 = false;
#else
    // Calculate SHA-256
    if (opt55 == true)
    {
        hashCtx_L.data.opt55hashCtx.dcpWork.control0 = 0x3042; // (uint32_t)kDCP_CONTROL0_HASH_INIT | (uint32_t)kDCP_CONTROL0_HASH_TERM | (kDCP_NoSwap & 0xFC0000u) | (uint32_t)kDCP_CONTROL0_ENABLE_HASH | (uint32_t)kDCP_CONTROL0_DECR_SEMAPHOR;
        hashCtx_L.data.opt55hashCtx.dcpWork.control1 = (uint32_t)kDCP_CONTROL1_HASH_SELECT_SHA256;
        hashCtx_L.data.opt55hashCtx.dcpWork.destinationBufferAddress = 0;
        hashCtx_L.data.opt55hashCtx.dcpWork.payloadPointer           = (uint32_t)(hashCtx_L.data.opt55hashCtx.runningHash);
        hashCtx_L.data.opt55hashCtx.dcpWork.nextCmdAddress = 0;
        hashCtx_L.data.opt55hashCtx.dcpWork.status = 0;
        hashCtx_L.typeOpt55 = true;
        status = kStatus_Success;
    }
    else
    {
        status = DCP_HASH_Init(DCP, &hashCtx_L.data.hashCtx.handle, &(hashCtx_L.data.hashCtx.ctx), kDCP_Sha256);
        hashCtx_L.typeOpt55 = false;
    }
#endif //(defined USE_BL_HOST) || (defined STRESS_FROM_HOST) || (defined BIN_LOADER)

    if (kStatus_Success != status)
    {
        return -1;
    }
    *ctx = (void*)(&hashCtx_L);
#else // USER_DEFINED_SHA256
    PLAT_SwSHA256_init(ctx);
#endif // USER_DEFINED_SHA256
    return 0;
}

int PLAT_SHA256_Init(void** ctx)
{
    return PLAT_SHA256_InitOpt(ctx, false);
}

int PLAT_SHA256_Update(void* ctx, const void* data, uint32_t dataSize)
{
#ifdef USER_DEFINED_SHA256
	PLAT_HASH_CTX_T* pHashCtx = (PLAT_HASH_CTX_T*)ctx;
    if (pHashCtx->typeOpt55 == false)
    {
    	status_t status;
#ifdef QLIB_PERF
    	CODE_SECTION_START(CODE_SECTION_HW_SHA);
#endif //QLIB_PERF
        status = DCP_HASH_Update(DCP, &pHashCtx->data.hashCtx.ctx , (uint8_t*)data, dataSize);
#ifdef QLIB_PERF
        CODE_SECTION_STOP(CODE_SECTION_HW_SHA);
#endif //QLIB_PERF
        if (kStatus_Success != status)
        {
            return -1;
         }
    }
    else
    {
        volatile uint32_t *cmdptr = &DCP->CH0CMDPTR;
        volatile uint32_t *chsema = &DCP->CH0SEMA;

    	// fixed sha 55 with 32b aligned data address
    	if ((dataSize != 55) || (((uint32_t)data & 0x3) != 0))
    	{
    		return -1;
    	}
#ifdef QLIB_PERF
        CODE_SECTION_START(CODE_SECTION_HW_SHA_FIX55);
#endif //QLIB_PERF

        // update data to be hashed
        pHashCtx->data.opt55hashCtx.dcpWork.sourceBufferAddress      = (uint32_t)data;
        pHashCtx->data.opt55hashCtx.dcpWork.bufferSize               = 55;

        // set out packet to DCP CMDPTR
        *cmdptr = (uint32_t)(&pHashCtx->data.opt55hashCtx.dcpWork);

        // Make sure that all data memory accesses are completed before starting of the job
        //__DSB();
        __ISB();

        // set the channel semaphore to start the job
        *chsema = 1u;

#ifdef QLIB_PERF
        CODE_SECTION_STOP(CODE_SECTION_HW_SHA_FIX55);
#endif //QLIB_PERF
    }

#else // USER_DEFINED_SHA256
    PLAT_SwSHA256_update(ctx, (const uint8_t*)data, dataSize);
#endif // USER_DEFINED_SHA256

    return 0;
}
int PLAT_SHA256_Finish(void* ctx, uint32_t* output)
{
#ifdef USER_DEFINED_SHA256
    PLAT_HASH_CTX_T* pHashCtx = (PLAT_HASH_CTX_T*)ctx;
    if (pHashCtx->typeOpt55 == false)
    {
        status_t status;
        size_t outputSize = HASH_SIZE;
#ifdef QLIB_PERF
        CODE_SECTION_START(CODE_SECTION_HW_SHA);
#endif //QLIB_PERF
        status = DCP_HASH_Finish(DCP, &pHashCtx->data.hashCtx.ctx, (uint8_t*)output, &outputSize);
#ifdef QLIB_PERF
        CODE_SECTION_STOP(CODE_SECTION_HW_SHA);
#endif //QLIB_PERF
        if (kStatus_Success != status)
        {
            memset (pHashCtx, 0, sizeof(PLAT_HASH_CTX_T));
            return -1;
         }
    }
    else
    {
#ifdef QLIB_PERF
        CODE_SECTION_START(CODE_SECTION_HW_SHA_FIX55);
#endif //QLIB_PERF
        volatile uint32_t *dcpStatClrPtr;

        /* wait if our channel is still active */
        while ((DCP->STAT & kDCP_Channel0) == kDCP_Channel0)
        {
        }

        // Clear status
        dcpStatClrPtr = (volatile uint32_t *)&DCP->STAT + 2u;
        *dcpStatClrPtr = 0xFFu;
        while ((DCP->STAT & 0xffu) != 0U)
        {
        }

        volatile uint32_t* runningHash = pHashCtx->data.opt55hashCtx.runningHash;
        // read output
        output[0] = __REV(runningHash[7]);
        output[1] = __REV(runningHash[6]);
        output[2] = __REV(runningHash[5]);
        output[3] = __REV(runningHash[4]);
        output[4] = __REV(runningHash[3]);
        output[5] = __REV(runningHash[2]);
        output[6] = __REV(runningHash[1]);
        output[7] = __REV(runningHash[0]);

#ifdef QLIB_PERF
        CODE_SECTION_STOP(CODE_SECTION_HW_SHA_FIX55);
#endif //QLIB_PERF
    }
#else // USER_DEFINED_SHA256
    PLAT_SwSHA256_final(ctx, (uint8_t*)output);
#endif // USER_DEFINED_SHA256

    return 0;
}
