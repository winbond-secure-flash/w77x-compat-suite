/************************************************************************************************************
* @internal
* @remark     Winbond Electronics Corporation
* @copyright  Copyright (c) 2023 by Winbond Electronics Corporation . All rights reserved
* @endinternal
*
* @file       comp_test_targets.h
* @brief      This file contains targets definitions
*
* ### project CompatibilityTests
*
************************************************************************************************************/
#ifndef _COMP_TEST_TARGETS_H__
#define _COMP_TEST_TARGETS_H__



/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                                DEFINITIONS                                              */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#define w77q128jw_revA (0x1u)
#define w77q64jw_revA  (0x4u)
#define w77q128jv_revA (0x8u)
#define w77q64jv_revA  (0x10u)
#define w77q32jw_revB  (0x20u)
#define w77q25nw_revB  (0x80u)
#define w77t25nw_revB  (0x100u)
#define w77q25nw_revD  (0x400u)
#define w77t25nw_revD  (0x800u)
#define w77q64nw_revA  (0x2000u)
#define w77t64nw_revA  (0x4000u)
#define w77q25nw1_revA (0x10000u)
#define w77t25nw1_revA (0x20000u)

#define all_q2_targets   (w77q128jw_revA | w77q64jw_revA | w77q128jv_revA | w77q64jv_revA | w77q32jw_revB)
#define all_q3_0_targets (w77q25nw_revB | w77t25nw_revB | w77q25nw_revD | w77t25nw_revD)
#define all_q3_1_targets (w77q64nw_revA | w77t64nw_revA | w77q25nw1_revA | w77t25nw1_revA)
#define q2_mcd_targets   (w77q32jw_revB)
#define q2_hcd18_targets (w77q128jw_revA | w77q64jw_revA)
#define q2_hcd33_targets (w77q128jv_revA | w77q64jv_revA)
#define q2_hcd_targets   (w77q128jw_revA | w77q64jw_revA | w77q128jv_revA | w77q64jv_revA)
//#define q3_t_targets     (w77t25nw_revB | w77t25nw_revD | w77t64nw_revA)
#define q3_octal_support_targets     (w77t25nw_revB | w77t25nw_revD | w77t64nw_revA)
#define all_q3_targets   (all_q3_0_targets | all_q3_1_targets)
#define all_targets      (all_q2_targets | all_q3_targets)

#ifndef TEST_TARGET
#define TEST_TARGET all_targets
#endif


#define TEST_TARGET_SUPPORTED(target) ((uint32_t)(target) & (uint32_t)TEST_TARGET)


#endif // _COMP_TEST_TARGETS_H__
