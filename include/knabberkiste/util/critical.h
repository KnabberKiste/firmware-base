/**
 * @file critical.h
 * @author Gabriel Heinzer
 * @brief Manages critical functinos which block interrupts from occurring.
 * Supports nesting of critical blocks.
 */

#pragma once

/**
 * @brief Enters a critical block. This means that interrupts are blocked
 * until @ref critical_exit() is called.
 */
void critical_enter();

/**
 * @brief Exits critical block. If this block was not wrapped in another
 * critical block, re-enabled interrupts.
 */
void critical_exit();
/**
 * @brief Shorthand for wrapping a code block in
 * @ref critical_enter and @ref critical_exit. Use curly braces as a delimeter.
 */
#define critical_block for(uint8_t __critical_dummy = 2; --__critical_dummy; critical_exit())