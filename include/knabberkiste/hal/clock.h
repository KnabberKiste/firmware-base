/**
 * @file clock.h
 * @author Gabriel Heinzer
 * @brief Contains utilities for controlling the clock tree of the microcontroller.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Configures the internal clock to run at 64 MHz.
 */
void clock_configure64MHz();

/**
 * @brief Gets the AHB prescaler relative to SYSCLK.
 * 
 * @return Prescaler of the AHB clock domain relative to SYSCLK.
 */
uint16_t clock_getAHBPrescaler();

/**
 * @brief Gets the APB1 precaler relative to SYSCLK.
 * 
 * @return Prescaler of the APB1 clock domain relative to SYSCLK.
 */
uint8_t clock_getAPB1Prescaler();

/**
 * @brief Gets the APB2 precaler relative to SYSCLK.
 * 
 * @return Prescaler of the APB2 clock domain relative to SYSCLK.
 */
uint8_t clock_getAPB2Prescaler();

/**
 * @brief Gets the clock freqency of the AHB clock domain.
 * 
 * @return Clock frequency in Hertz (s^-1) of the AHB clock domain.
 */
uint32_t clock_getAHBFrequency();

/**
 * @brief Gets the clock freqency of the APB1 clock domain.
 * 
 * @return Clock frequency in Hertz (s^-1) of the APB1 clock domain.
 */
uint32_t clock_getAPB1Frequency();

/**
 * @brief Gets the clock freqency of the APB2 clock domain.
 * 
 * @return Clock frequency in Hertz (s^-1) of the APB2 clock domain.
 */
uint32_t clock_getAPB2Frequency();