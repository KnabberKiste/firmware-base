#pragma once

#define SET_MASK(target, mask) ((target) |= (mask))
#define CLEAR_MASK(target, mask) ((target) &= ~(mask))
#define TOGGLE_MASK(target, mask) ((target) ^= (mask))
#define READ_MASK(target, mask) ((target) & (mask))
#define WRITE_MASK(target, mask, value) (((target) = ((target) & ~(mask)) | ((value) & (mask))))

#define SET_MASK_OFFSET(target, mask, offset) SET_MASK((target), (mask) << (offset))
#define CLEAR_MASK_OFFSET(target, mask, offset) CLEAR_MASK((target), (mask) << (offset))
#define TOGGLE_MASK_OFFSET(target, mask, offset) TOGGLE_MASK((target), (mask) << (offset))
#define READ_MASK_OFFSET(target, mask, offset) (READ_MASK((target), (mask) << (offset)) >> (offset))
#define WRITE_MASK_OFFSET(target, mask, value, offset) WRITE_MASK((target), (mask) << (offset), (value) << (offset))

#define SET_BIT(target, offset) (SET_MASK_OFFSET((target), 1, (offset)))
#define CLEAR_BIT(target, offset) (CLEAR_MASK_OFFSET((target), 1, (offset)))
#define TOGGLE_BIT(target, offset) (TOGGLE_MASK_OFFSET((target), 1, (offset)))
#define READ_BIT(target, offset) (READ_MASK_OFFSET((target), 1, (offset)))
#define WRITE_BIT(target, offset, value) (WRITE_MASK_OFFSET((target), 1, (value), (offset)))