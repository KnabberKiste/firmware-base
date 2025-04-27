/**
 * @file io.h
 * @author Gabriel Heinzer
 * @brief Provides the CMSIS include header for the used chip (STM32F303xC). This
 * also provides some Cortex- and STM32-specific macros.
 */

#pragma once

#include <stm32f303xc.h>
#include <knabberkiste/util/bit_manipulation.h>

/**
 * @brief Provides the currently active interrupt vector, or 0 if in
 * thread mode.
 */
#define CORTEX_ACTIVE_INTERRUPT_VECTOR (READ_MASK(SCB->ICSR, SCB_ICSR_VECTACTIVE_Msk))