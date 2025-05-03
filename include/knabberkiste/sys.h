/**
 * @file sys.h
 * @author Gabriel Heinzer
 * @brief Provides a system initialization function.
 */

#pragma once

/**
 * @brief Convenience function for system initialization. 
 * 
 * Initializes the whole system, performing the following steps:
 * 
 * 1. Set SYSCLK to 64 MHz
 * 2. Enable all GPIO port clocks
 * 3. Initialize the VCP interface to 921600 baud
 * 4. Initialize the CAN bus
 */
void sys_init();