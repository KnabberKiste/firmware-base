/**
 * @file wdg.h
 * @author Gabriel Heinzer
 * @brief HAL for accessing the IWDG (independent watchdog).
 */

#pragma once

/**
 * @brief Initializes the independent watchdog (IWDG) with the specified
 * timeout.
 * 
 * @param timeout The timeout value after which the watchdog will reset the
 * microcontroller.
 */
void iwdg_init(uint32_t timeout);

/**
 * @brief Resets the independent watchdog. This will cause the downcounter to
 * be reset. You **must** call this regularly to ensure the IWDG will not time
 * out and therefore reset the microcontroller.
 */
void iwdg_reset();