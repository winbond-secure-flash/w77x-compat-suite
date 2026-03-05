/************************************************************************************************************
* @internal
* @copyright  Copyright (c) 2025 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_errors.h
* @brief      This file contains error handling definitions
*
* ### project Compatibility Tests
*
************************************************************************************************************/
#ifndef _COMP_TEST_ERRORS_H_
#define _COMP_TEST_ERRORS_H_
#include "defs.h"
#include "stdio.h"

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                             VERBOSITY LEVELS                                            */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define COMP_TEST_VERBOSE_NONE    0
#define COMP_TEST_VERBOSE_FATAL   1
#define COMP_TEST_VERBOSE_ERROR   2
#define COMP_TEST_VERBOSE_WARNING 3
#define COMP_TEST_VERBOSE_INFO    4
#define COMP_TEST_VERBOSE_DEBUG   5
#define COMP_TEST_VERBOSE_TRACE   6
#define COMP_TEST_VERBOSE_ALL     7
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       DEFAULT VERBOSITY SELECTION                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifndef COMP_TEST_VERBOSE
#define COMP_TEST_VERBOSE COMP_TEST_VERBOSE_INFO
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                         ERROR CODES DEFINITIONS                                         */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

#define _COMP_TEST_STATUS(ELEMENT)                              \
    {                                                           \
        ELEMENT(COMP_TEST_STATUS_OK),                           \
        ELEMENT(COMP_TEST_STATUS_FAIL),                         \
                                                                \
        ELEMENT(COMP_TEST_STATUS__INVALID_PARAMETER),           \
        ELEMENT(COMP_TEST_STATUS__COMMAND_FAIL),                \
        ELEMENT(COMP_TEST_STATUS__SYSTEM_IN_INCORRECT_STATE),   \
        ELEMENT(COMP_TEST_STATUS__NOT_CONNECTED),               \
        ELEMENT(COMP_TEST_STATUS__PARAMETER_OUT_OF_RANGE),      \
        ELEMENT(COMP_TEST_STATUS__INVALID_DATA_SIZE),           \
        ELEMENT(COMP_TEST_STATUS__INVALID_DATA_ALIGNMENT),      \
        ELEMENT(COMP_TEST_STATUS__COMMUNICATION_ERR),           \
        ELEMENT(COMP_TEST_STATUS__SECURITY_ERR),                \
                                                                \
        /* Implementation State */                              \
        ELEMENT(COMP_TEST_STATUS__NOT_IMPLEMENTED),             \
        ELEMENT(COMP_TEST_STATUS__NOT_SUPPORTED),               \
        ELEMENT(COMP_TEST_STATUS__OBSOLETE_FUNCTION),           \
                                                                \
        /* HW errors */                                         \
        ELEMENT(COMP_TEST_STATUS__COMMAND_IGNORED),             \
        ELEMENT(COMP_TEST_STATUS__DEVICE_BUSY),                 \
        ELEMENT(COMP_TEST_STATUS__DEVICE_ERR),                  \
        ELEMENT(COMP_TEST_STATUS__DEVICE_ERR_MULTI),            \
        ELEMENT(COMP_TEST_STATUS__DEVICE_SESSION_ERR),          \
        ELEMENT(COMP_TEST_STATUS__DEVICE_INTEGRITY_ERR),        \
        ELEMENT(COMP_TEST_STATUS__DEVICE_AUTHENTICATION_ERR),   \
        ELEMENT(COMP_TEST_STATUS__DEVICE_PRIVILEGE_ERR),        \
        ELEMENT(COMP_TEST_STATUS__DEVICE_SYSTEM_ERR),           \
        ELEMENT(COMP_TEST_STATUS__DEVICE_FLASH_ERR),            \
        ELEMENT(COMP_TEST_STATUS__DEVICE_MC_ERR),               \
        ELEMENT(COMP_TEST_STATUS__HARDWARE_FAILURE),            \
        ELEMENT(COMP_TEST_STATUS__TIME_OUT),                    \
        ELEMENT(COMP_TEST_STATUS__CONNECTIVITY_ERR),            \
        ELEMENT(COMP_TEST_STATUS__OUT_OF_MEMORY),               \
        ELEMENT(COMP_TEST_STATUS__ECC_DOUBLE_ERROR_DETECTION),  \
                                                                \
        ELEMENT(COMP_TEST_STATUS_TRANSPORT_SDR_FAIL),           \
        ELEMENT(COMP_TEST_STATUS_TRANSPORT_QUAD_FAIL),           \
        ELEMENT(COMP_TEST_STATUS_TRANSPORT_OCT_FAIL),           \
        ELEMENT(COMP_TEST_STATUS_SET_SECURE_REG_FAIL),          \
                                                                \
        ELEMENT(COMP_TEST_STATUS_LAST)                          \
    }
// clang-format on

typedef enum _COMP_TEST_STATUS(GENERATE_ENUM) COMP_TEST_STATUS_T;

#define COMP_TEST_STATUS_STR_TAB _COMP_TEST_STATUS(GENERATE_STRING)

#ifdef COMP_TEST_DEBUG
extern const char* compTestStatusStrings[];

#define STATUS_TO_STR(status) (status < COMP_TEST_STATUS_LAST ? compTestStatusStrings[status] : "Unknown status")
#else
#define STATUS_TO_STR(status) ""
#endif //COMP_TEST_DEBUG



#define COMP_TEST_STATUS_RET_CHECK(func)                                                                      \
    {                                                                                                         \
        COMP_TEST_STATUS_T ___ret;                                                                            \
        if (COMP_TEST_STATUS_OK != (___ret = func))                                                           \
        {                                                                                                     \
            return ___ret;                                                                                    \
        }                                                                                                     \
    }


#if defined(DEBUG) || defined(_DEBUG)
    #ifndef COMP_TEST_PRINT_CMD
        #define COMP_TEST_PRINT_CMD(...) \
            printf(__VA_ARGS__);    \
            fflush(stdout);
    #endif // COMP_TEST_PRINT_CMD
#else  // defined(DEBUG) || defined(_DEBUG)
    #ifdef COMP_TEST_PRINT_CMD
        #undef COMP_TEST_PRINT_CMD
    #endif // COMP_TEST_PRINT_CMD
    #define COMP_TEST_PRINT_CMD(...)
#endif // defined(DEBUG) || defined(_DEBUG)


#define COMP_TEST_PRINT(...)          \
{                                     \
    COMP_TEST_PRINT_CMD(__VA_ARGS__); \
    COMP_TEST_PRINT_CMD("\r\n");      \
}

#define COMP_TEST_DEBUG_PRINT(verbose, ...)  \
{                                            \
        if (COMP_TEST_VERBOSE >= verbose)    \
        {                                    \
            COMP_TEST_PRINT(__VA_ARGS__);    \
        }                                    \
}

#define COMP_TEST_ASSERT_RET(cond, err)                                                                       \
    {                                                                                                         \
        if (!(cond))                                                                                          \
        {                                                                                                     \
            COMP_TEST_DEBUG_PRINT(COMP_TEST_VERBOSE_ERROR, " ASSERT [%d %s] in: %s", (err), STATUS_TO_STR(err), #cond); \
            return err;                                                                                       \
        }                                                                                                     \
    }




#endif // _COMP_TEST_ERRORS_H_
