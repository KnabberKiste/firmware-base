/**
 * @file vcp_debug.h
 * @author Gabriel Heinzer
 * @brief Provides debugging support for VCP (virtual COM port).
 * 
 * @details All functions in this file use the virtual COM port from the ST-link debugger.
 */

#pragma once

/**
 * @brief Initializes the serial port with the given baud rate.
 * 
 * @param baudrate The baudrate, in baud, to initialize the interface with.
 */
void vcp_init(long long baudrate);

/**
 * @brief Prints a single char to the VCP interface.
 * 
 * @param c The character to print.
 */
void vcp_putchar(char c);

/**
 * @brief Prints the given string to the VCP interface.
 * 
 * @param str The string to print to the VCP.
 */
void vcp_print(const char* str);

/**
 * @brief Prints the given string to the VCP interface with a trailing newline.
 * 
 * @param str The string to print to the VCP.
 */
void vcp_println(const char* str);