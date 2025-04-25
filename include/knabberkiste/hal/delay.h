/**
 * @file delay.h
 * @author Gabriel Heinzer
 * @brief Delay library for environments without FreeRTOS.
 * 
 * @warning DO NOT use this when using FreeRTOS, use ``vTaskDelay`` instead. This
 * library raises a preprocessor error when using together with FreeRTOS.
 */

#pragma once

#if __has_include("FreeRTOS.h") && __DOXYGEN__ != 1

    #error You seem to be using FreeRTOS, so please use that and not this knabberkiste/util/delay.h for your delay needs.

#else

    /**
     * @brief Enumerator of tick rates.
     * 
     */
    typedef enum {
        /// @brief Tick rate = 10 Hz, one tick every 100 ms.
        DELAYRES_100MS = 10,
        /// @brief Tick rate = 100 Hz, one tick every 10 ms.
        DELAYRES_10MS = 100,
        /// @brief Tick rate = 1 kHz, one tick every millisecond.
        DELAYRES_1MS = 1000,
        /// @brief Tick rate = 10 kHz, one tick every 100 us.
        DELAYRES_100US = 10000
    } TickRate_t;

    /**
     * @brief Initialize the delay library.
     * 
     * @param res Tick rate to use for the delays. Choose a tick rate
     * that is appropriate for your needs, i.e. as slow as possible,
     * but as fast as required.
     */
    void delay_init(TickRate_t res);

    /**
     * @brief Delays the execution for the specified amount of milliseconds.
     * You may use a fraction of milliseconds, but that may only have an effect
     * if using an appropriate tick rate.
     * 
     * @warning You must first initialize the delay library using @ref delay_init().
     * 
     * @see @ref delay_init()
     * 
     * @param milliseconds 
     */
    void delay(float milliseconds);

#endif