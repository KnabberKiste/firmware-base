/**
 * @file macro_util.h
 * @author Gabriel Heinzer
 * @brief Various preprocessor macro utilites.
 */

#pragma once

#define _TOKEN_CONCAT(a, b) a ## b

/**
 * @brief Concatenates the tokens @p a and @p b in that order to a single token.
 */
#define TOKEN_CONCAT(a, b) _TOKEN_CONCAT(a, b)