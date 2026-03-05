/*---------------------------------------------------------------------------------------------------------*/
/* Winbond Electronics Corporation                                                                         */
/*                                                                                                         */
/*                                                                                                         */
/* Copyright (c) 2019 by Winbond Electronics Corporation                                                   */
/* All rights reserved                                                                                     */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
/* File Content:                                                                                           */
/*            This file contains NXP 10xx interrupts interface for interrupts on NXP 10xx device           */
/* Project:                                                                                                */
/*            Q2                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

/************************************************************************************************************
 * @brief       This macro declare variable to store interrupt state
 * @param  interrupts   variable name that is declared to store the interrupt state
************************************************************************************************************/
#define INTERRUPTS_VAR_DECLARE(interrupts)      uint32_t interrupts

/************************************************************************************************************
 * @brief       This macro preserves interrupt state and disable interrupts - Used in the beginning of an atomic flow
 * @param  interrupts   the variable declared in @ref INTERRUPTS_VAR_DECLARE which will store the current enabled interrupts
************************************************************************************************************/
#define INTERRUPTS_SAVE_DISABLE(interrupts)     interrupts = DisableGlobalIRQ();

/************************************************************************************************************
 * @brief       This macro restores the saved interrupt state from @ref INTERRUPTS_SAVE_DISABLE. Used at the end of an atomic flow
 * @param  interrupts   the variable declared in @ref INTERRUPTS_VAR_DECLARE which used in @ref INTERRUPTS_SAVE_DISABLE and stored the enabled interrupts
************************************************************************************************************/
#define INTERRUPTS_RESTORE(interrupts)          EnableGlobalIRQ(interrupts)

#endif // __INTERRUPT_H__
