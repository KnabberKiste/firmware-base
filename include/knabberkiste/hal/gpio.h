/**
 * @file gpio.h
 * @author Gabriel Heinzer
 * @brief HAL for the general purpose inputs and outputs (GPIO).
 */

#pragma once

#include <knabberkiste/io.h>
#include <stdbool.h>

/**
 * @brief Non-transparent data type defining a GPIO pin. All the pins are pre-defined in the
 * @ref gpio.h file, you shouldn't need to define any.
 */
typedef struct GPIO_Pin GPIO_Pin_t;

/**
 * @brief Enumeration of GPIO connectivity test results.
 */
typedef enum {
    /// @brief The GPIO seems to be tied to a high-impedance path.
    GPIO_CT_FLOATING,
    /// @brief The GPIO seems to be tied to a low-impedance path to ground.
    GPIO_CT_TIED_LOW,
    /// @brief The GPIO seems to be tied to a low-impedance path to VCC.
    GPIO_CT_TIED_HIGH
} GPIO_ConnectivityTestResult_t;

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
 * @brief Enables the port clock of a GPIO pin. This must be done before
 * accessing any functionality on the given pin, as that may fail otherwise.
 * 
 * @param pin Pin to activate the port clock on.
 */
void gpio_enable_port_clock(GPIO_Pin_t* pin);

/**
 * @brief Set the mode of the specified GPIO pin.
 * 
 * @param pin The pin you want to change the mode for.
 * @param mode The mode you want to assign to the pin.
 */
void gpio_set_pin_mode(GPIO_Pin_t* pin, GPIO_Mode_t mode);
/**
 * @brief Sets the output type of the specified GPIO pin. Only
 * applicable to pins configured as @ref GPIO_MODE_OUTPUT.
 * 
 * @see @ref gpio_set_pin_mode()
 * 
 * @param pin Pin to configure the output type for.
 * @param type Output type to set the GPIO pin to.
 */
void gpio_set_output_type(GPIO_Pin_t* pin, GPIO_OutputType_t type);
/**
 * @brief Sets the pull configuration of the specified GPIO pin. Only
 * applicable to pins configured as @ref GPIO_MODE_INPUT.
 * 
 * @see @ref gpio_set_pin_mode()
 * 
 * @param pin Pin to set the pull configuration for.
 * @param type Pull configuartino you want to configure for the pin.
 */
void gpio_set_pull_configuration(GPIO_Pin_t* pin, GPIO_PullConfiguration_t type);
/**
 * @brief Assigns an alternate function to the specified GPIO pin. Only
 * applicable to pins configured as @ref GPIO_MODE_ALTERNATE.
 * 
 * @see @ref gpio_set_pin_mode()
 * 
 * @param pin The pin you want to configure the alternate function for.
 * @param alternate The alternate funtion you want to assign to the pin.
 */
void gpio_set_alternate(GPIO_Pin_t* pin, GPIO_AlternateFunction_t alternate);

/**
 * @brief Reads the value from the given GPIO pin.
 * 
 * @param pin The pin you want to read the value from.
 * @retval true The voltage on the input pin exceeds the input high voltage VIH.
 * @retval false The voltaeg on the input pin is lower than the input low voltage VIL.
 */
bool gpio_read_pin(GPIO_Pin_t* pin);
/**
 * @brief Writes the given boolean value to the specified GPIO pin.
 * 
 * This is only applicable to pins set to @ref GPIO_MODE_OUTPUT.
 * 
 * @param pin The pin you want to assign the boolean value to.
 * @param value The boolean value you want to write to the pin.
 */
void gpio_write_pin(GPIO_Pin_t* pin, bool value);
/**
 * @brief Sets the given output pin to HIGH.
 * 
 * This is only applicable to pins set to @ref GPIO_MODE_OUTPUT.
 * 
 * @param pin The pin you want to set to HIGH.
 */
void gpio_set_pin(GPIO_Pin_t* pin);
/**
 * @brief Clears the given output pin, setting it to LOW.
 * 
 * This is only applicable to pins set to @ref GPIO_MODE_OUTPUT.
 * 
 * @param pin The pin you want to clear.
 */
void gpio_clear_pin(GPIO_Pin_t* pin);
/**
 * @brief Toggles the given output pin, setting it to the inverse
 * of it's previous state.
 * 
 * This is only applicable to pins set to @ref GPIO_MODE_OUTPUT.
 * 
 * @param pin The pin you want to toggle.
 */
void gpio_toggle_pin(GPIO_Pin_t* pin);

/**
 * @brief Attempts to check what is connected to the given GPIO pin by
 * checking what input values it reads as when enabling and disabling
 * the pull-up or pull-down resistors.
 * 
 * @warning This method might be flawed, but you can use this as a sanity
 * check on your board to check for solder bridges and such.
 * 
 * @param pin The pin you want to test the connectivity of.
 * @return The connectivity result of the test.
 */
GPIO_ConnectivityTestResult_t gpio_test_connectivity(GPIO_Pin_t* pin);

// GPIO pin definitions
#define __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, pin) extern GPIO_Pin_t* P##portLetter##pin
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

__GPIO_SINGLE_PORT_DEFINITION(A, 0);
__GPIO_SINGLE_PORT_DEFINITION(B, 1);
__GPIO_SINGLE_PORT_DEFINITION(C, 2);
__GPIO_SINGLE_PORT_DEFINITION(D, 3);
__GPIO_SINGLE_PORT_DEFINITION(E, 4);
__GPIO_SINGLE_PORT_DEFINITION(F, 5);

#undef __GPIO_SINGLE_PIN_DEFINITON
#undef __GPIO_SINGLE_PORT_DEFINITION