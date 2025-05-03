/**
 * @file gpio.h
 * @author Gabriel Heinzer
 * @brief HAL for the general purpose inputs and outputs (GPIO).
 */

#pragma once

#include <knabberkiste/io.h>
#include <stdbool.h>

/**
 * @brief Enumeration of GPIO pin modes.
 */
typedef enum {
    /**
     * @brief Reads digital signals from a pin.
     * 
     * @see You can set the pull-mode of the input pin using gpio_set_pull_configuration().
     */
    GPIO_MODE_INPUT = 0b00,
    /**
     * @brief Writes digital signals to a pin.
     * 
     * @see You can set the mode of the output using gpio_set_output_type().
     */
    GPIO_MODE_OUTPUT = 0b01,
    /**
     * @brief Assigns the pin to one of the alternate functions. This can be
     * used for example if using an internal communication interface.
     * 
     * @see You can set the altenate function of the GPIO pin using gpio_set_alternate().
     */
    GPIO_MODE_ALTERNATE = 0b10,
    /**
     * @brief High-impedance mode used for analog-to-digital-conversion.
     */
    GPIO_MODE_ANALOG = 0b11
} GPIO_Mode_t;

/**
 * @brief Enumeration of GPIO output types.
 */
typedef enum {
    /// @brief Push-pull type output (default).
    GPIO_OUTPUT_PUSHPULL = 0,
    /// @brief Open-drain output type.
    GPIO_OUTPUT_OPENDRAIN = 1
} GPIO_OutputType_t;

/**
 * @brief Enumeration of GPIO output speeds.
 */
typedef enum {
    /// @brief Low speed output.
    GPIO_OUTPUT_SPEED_LOW = 0,
    /// @brief Medium speed output.
    GPIO_OUTPUT_SPEED_MEDIUM = 0b01,
    /// @brief High speed output.
    GPIO_OUTPUT_SPEED_HIGH = 0b11,
} GPIO_OutputSpeed_t;

/**
 * @brief Enumeration of GPIO input pull configurations.
 */
typedef enum {
    /// @brief Don't activate a pull-up or pull-down resistor (high-impedance, default).
    GPIO_NOPULL = 0b00,
    /// @brief Activate the internal pull-up resistor.
    GPIO_PULLUP = 0b01,
    /// @brief Activate the internal pull-down resistor.
    GPIO_PULLDOWN = 0b10
} GPIO_PullConfiguration_t;

/**
 * @brief Enumeration of GPIO alternate functions.
 */
typedef enum {
    GPIO_AF0 = 0,
    GPIO_AF1 = 1,
    GPIO_AF2 = 2,
    GPIO_AF3 = 3,
    GPIO_AF4 = 4,
    GPIO_AF5 = 5,
    GPIO_AF6 = 6,
    GPIO_AF7 = 7,
    GPIO_AF8 = 8,
    GPIO_AF9 = 9,
    GPIO_AF10 = 10,
    GPIO_AF11 = 11,
    GPIO_AF12 = 12,
    GPIO_AF13 = 13,
    GPIO_AF14 = 14,
    GPIO_AF15 = 15
} GPIO_AlternateFunction_t;

/**
 * @brief Enables the port clocks for all GPIO ports.
 */
void gpio_enable_port_clocks();

// GPIO pin definitions
#define __GPIO_PIN_TYPE_DEFINITION(pin_id) \
    struct __attribute__((packed)) __GPIO_PinType##pin_id { \
        volatile uint32_t BITFIELD_SELECT(pin_id, mode, 2, 32); \
        volatile uint32_t BITFIELD_SELECT(pin_id, output_type, 1, 32); \
        volatile uint32_t BITFIELD_SELECT(pin_id, output_speed, 2, 32); \
        volatile uint32_t BITFIELD_SELECT(pin_id, pull_mode, 2, 32); \
        volatile uint32_t BITFIELD_SELECT(pin_id, input_data, 1, 32); \
        volatile uint32_t BITFIELD_SELECT(pin_id, output_data, 1, 32); \
        volatile uint16_t BITFIELD_SELECT(pin_id, set, 1, 16); \
        volatile uint16_t BITFIELD_SELECT(pin_id, reset, 1, 16); \
        volatile uint32_t : 32; \
        volatile uint64_t BITFIELD_SELECT(pin_id, alternate, 4, 64); \
        volatile uint32_t : 32; \
    };

#define __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, pin) extern struct __GPIO_PinType##pin* const P##portLetter##pin;;
#define __GPIO_SINGLE_PORT_DEFINITION(portLetter, portOffset) \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 0); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 1); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 2); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 3); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 4); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 5); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 6); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 7); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 8); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 9); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 10); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 11); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 12); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 13); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 14); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 15)

__GPIO_PIN_TYPE_DEFINITION(0); \
__GPIO_PIN_TYPE_DEFINITION(1); \
__GPIO_PIN_TYPE_DEFINITION(2); \
__GPIO_PIN_TYPE_DEFINITION(3); \
__GPIO_PIN_TYPE_DEFINITION(4); \
__GPIO_PIN_TYPE_DEFINITION(5); \
__GPIO_PIN_TYPE_DEFINITION(6); \
__GPIO_PIN_TYPE_DEFINITION(7); \
__GPIO_PIN_TYPE_DEFINITION(8); \
__GPIO_PIN_TYPE_DEFINITION(9); \
__GPIO_PIN_TYPE_DEFINITION(10); \
__GPIO_PIN_TYPE_DEFINITION(11); \
__GPIO_PIN_TYPE_DEFINITION(12); \
__GPIO_PIN_TYPE_DEFINITION(13); \
__GPIO_PIN_TYPE_DEFINITION(14); \
__GPIO_PIN_TYPE_DEFINITION(15)

__GPIO_SINGLE_PORT_DEFINITION(A, 0);
__GPIO_SINGLE_PORT_DEFINITION(B, 1);
__GPIO_SINGLE_PORT_DEFINITION(C, 2);
__GPIO_SINGLE_PORT_DEFINITION(D, 3);
__GPIO_SINGLE_PORT_DEFINITION(E, 4);
__GPIO_SINGLE_PORT_DEFINITION(F, 5);

#undef __GPIO_SINGLE_PIN_DEFINITON
#undef __GPIO_SINGLE_PORT_DEFINITION