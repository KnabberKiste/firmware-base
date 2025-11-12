/**
 * @file st7066u.h
 * @author Gabriel Heinzer (gabriel.heinzer@roche.com)
 * @brief Driver library for the SITRONIX ST7066U Dot Matrix LCD Controller/Driver.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __DOXYGEN__

    /**
    * @brief Application-defined macro which expands to a GPIO definition.
    *
    * This must be configured in a file named `st7066u_config.h` in the include path.
    * 
    * Configure this to the matching pin on the ST7066U.
    */    
    #define ST7066U_GPIO_RW /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_E /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_RS /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB0 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB1 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB2 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB3 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB4 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB5 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB6 /* application-specific */
    /**
     * @brief Application-defined macro which expands to a GPIO definition.
     *
     * This must be configured in a file named `st7066u_config.h` in the include path.
     * 
     * Configure this to the matching pin on the ST7066U.
    */
    #define ST7066U_GPIO_DB7 /* application-specific */

#endif

/**
 * @brief Directions supported by the ST7066U driver.
 */
typedef enum {
    ST7066U_BACKWARD = 0,
    ST7066U_FORWARD = 1
} ST7066U_MoveDirection_t;

/**
 * @brief Initializes the ST7066U driver.
 */
void st7066u_init();

/**
 * @brief Writes a single byte to the ST7066U's RAM.
 * 
 * @param byte Byte to write.
 */
void st7066u_write_byte(uint8_t byte);
/**
 * @brief Writes a number of bytes to the ST7066U's RAM.
 * 
 * @param buf Pointer to a buffer of data to write.
 * @param size Size of the buffer to write.
 */
void st7066u_write_chunk(uint8_t* buf, size_t size);
/**
 * @brief Writes a string to the ST7066U's RAM.
 * 
 * @param str String to write (null-terminated).
 */
void st7066u_write_string(char* str);

/**
 * @brief Writes a single command to the ST7066U.
 * 
 * @param command Command byte to write.
 */
void st7066u_write_command(uint8_t command);

/**
 * @brief Clears the ST7066U's display.
 * 
 */
void st7066u_clear_display();
/**
 * @brief Sets the DDRAM address of the ST7066U to 0x00 and returns the cursor to it's original position.
 * 
 */
void st7066u_return_home();
/**
 * @brief Sets the operation which is applied when data is written to the ST7066U's DDRAM.
 * 
 * @param direction Direction in which the address is moved, either decremented or incremented.
 * @param display_shift Whether the whole display should be shifted when a byte is written. This makes it look as if the cursor stands still and the display moves when each character is written to DDRAM.
 */
void st7066u_set_entry_mode(ST7066U_MoveDirection_t direction, bool display_shift);
/**
 * @brief Turns individual features of the ST7066U's display on or off.
 * 
 * @param display Whether the display as a whole is enabled.
 * @param cursor Whether the cursor should be enabled.
 * @param cursor_blinking Whether the cursor should be blinking.
 */
void st7066u_on_off_control(bool display, bool cursor, bool cursor_blinking);
/**
 * @brief Shifts the cursor of the display to the specified direction by one character.
 * 
 * @param direction Direction in which the cursor should be shifted.
 */
void st7066u_shift_cursor(ST7066U_MoveDirection_t direction);
/**
 * @brief Shifts the entire display data to the specified direction by one character.
 * 
 * @param direction Direction in which the display should be shifted.
 */
void st7066u_shift_display(ST7066U_MoveDirection_t direction);
/**
 * @brief Puts the display into CGRAM mode and sets the address of the pointer.
 * 
 * @param address CGRAM Address to set the pointer to.
 */
void st7066u_set_cgram_address(uint8_t address);
/**
 * @brief Puts the display into DDRAM mode and sets the address of the pointer.
 * 
 * @param address DDRAM address to set the pointer to.
 */
void st7066u_set_ddram_address(uint8_t address);