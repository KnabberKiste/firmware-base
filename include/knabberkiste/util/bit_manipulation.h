/**
 * @file bit_manipulation.h
 * @author Gabriel Heinzer
 * @brief Various utility macros for bit manipulation.
 */

#pragma once

/// @brief Sets all the bits in @p mask in the @p target.
#define SET_MASK(target, mask) ((target) |= (mask))
/// @brief Clears all the bits in @p mask in the @p target.
#define CLEAR_MASK(target, mask) ((target) &= ~(mask))
/// @brief Toggles all the bits in @p mask in the @p target.
#define TOGGLE_MASK(target, mask) ((target) ^= (mask))
/// @brief Reads all the bits in @p mask in the @p target.
#define READ_MASK(target, mask) ((target) & (mask))
/// @brief Writes all the bits in @p mask to their states in @p value to the @p target.
#define WRITE_MASK(target, mask, value) (((target) = ((target) & ~(mask)) | ((value) & (mask))))

/// @brief Same as @ref SET_MASK, but accounting for an offset.
#define SET_MASK_OFFSET(target, mask, offset) SET_MASK((target), (mask) << (offset))
/// @brief Same as @ref CLEAR_MASK, but accounting for an offset.
#define CLEAR_MASK_OFFSET(target, mask, offset) CLEAR_MASK((target), (mask) << (offset))
/// @brief Same as @ref TOGGLE_MASK, but accounting for an offset.
#define TOGGLE_MASK_OFFSET(target, mask, offset) TOGGLE_MASK((target), (mask) << (offset))
/// @brief Same as @ref READ_MASK, but accounting for an offset.
#define READ_MASK_OFFSET(target, mask, offset) (READ_MASK((target), (mask) << (offset)) >> (offset))
/// @brief Same as @ref WRITE_MASK, but accounting for an offset.
#define WRITE_MASK_OFFSET(target, mask, value, offset) WRITE_MASK((target), (mask) << (offset), (value) << (offset))

/// @brief Sets the bit at offset @p offset in the @p target.
#define SET_BIT(target, offset) (SET_MASK_OFFSET((target), 1, (offset)))
/// @brief Clears the bit at offset @p offset in the @p target.
#define CLEAR_BIT(target, offset) (CLEAR_MASK_OFFSET((target), 1, (offset)))
/// @brief Toggles the bit at offset @p offset in the @p target.
#define TOGGLE_BIT(target, offset) (TOGGLE_MASK_OFFSET((target), 1, (offset)))
/// @brief Reads the bit at offset @p offset in the @p target.
#define READ_BIT(target, offset) (READ_MASK_OFFSET((target), 1, (offset)))
/// @brief Writes the bit at offset @p offset in the @p target.
#define WRITE_BIT(target, offset, value) (WRITE_MASK_OFFSET((target), 1, (value), (offset)))

/**
 * @brief Bitfield selection macro. Creates the code for selecting the bitfield with the specified
 * @p size at the @p offset in a register with size @p regsize. The bitfield will be named with the given
 * @p name.
 */
#define BITFIELD_SELECT(offset, name, size, regsize) : ((offset) * (size)), name : (size), : ((regsize) - (((offset) + 1) * (size)))