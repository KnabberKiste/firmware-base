/**
 * @file error.h
 * @author Gabriel Heinzer
 * @brief Thread-safe error management utility.
 * 
 * Implements a try-/catch-block using ``setjmp`` and ``longjmp``. This is also
 * compatible with FreeRTOS. When using with FreeRTOS, only the current task
 * is affected from uncaught erors.
 * 
 * @code{.c}
 * error_try {
 *     // Error-prone code here
 * } error_catch(error_t somevar) {
 *     // Handle your error here
 * } error_end_handling;
 * @endcode
 */

#pragma once

#include <knabberkiste/io.h>
#include <knabberkiste/util/macro_util.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Error code type used in @ref util/error.h
 */
typedef uint8_t error_code_t;

/**
 * @brief Struct representing an error which may occur, or may
 * have occurred.
 */
typedef struct {
    /// @brief Error code.
    error_code_t error_code;
    /// @brief String-type error message passed with the error code.
    const char* error_message;
} error_t;

/**
 * @internal
 * @brief Struct representing the state of the error manager for
 * any given task.
 */
typedef struct {
    jmp_buf try_buf;
    bool try_active;
    error_t current_error;
} __error_manager_state_t;

/**
 * @internal
 * @brief Error manager state of the default task.
 */
extern volatile __error_manager_state_t __error_manager_state;

/**
 * @brief Throws an error with the given error information.
 * 
 * @param error_code Error code to throw.
 * @param error_message Error message to pass along with the error code.
 */
void error_throw(error_code_t error_code, const char* error_message);

#if __has_include("FreeRTOS.h")
    /* FreeRTOS implementation*/
    #include <FreeRTOS.h>
    #include <task.h>

    #define __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX 0

    #define __get_em_ptr \
        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { \
            /* FreeRTOS scheduler is running */ \
            __em_ptr = pvTaskGetThreadLocalStoragePointer(NULL, __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX); \
            \
            /* Allocate the error manager state struct if not already done. */ \
            if(__em_ptr == 0) { \
                /* The error manager state struct has to be allocated. */ \
                __em_ptr = pvPortMalloc(sizeof(__error_manager_state_t)); \
                vTaskSetThreadLocalStoragePointer(NULL, __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX, __em_ptr); \
            } \
        } else { \
            /* FreeRTOS scheduler is not running */ \
            __em_ptr = &__error_manager_state; \
        }

#else

    #define __get_em_ptr \
        __em_ptr = &__error_manager_state; \

#endif

/**
 * @brief Try block of the error library.
 */
#define error_try \
    { \
        volatile __error_manager_state_t* __em_ptr; \
        int __setjmp_result = 0; \
        \
        __get_em_ptr; \
        \
        /* Store the previous state. */ \
        __error_manager_state_t __previous_error_manager_state = *__em_ptr; \
        \
        /* Set the current state */ \
        __disable_irq(); \
        __em_ptr->try_active = true; \
        __setjmp_result = setjmp(__em_ptr->try_buf); \
        __enable_irq(); \
        if(__setjmp_result == 0)
        

/**
 * @brief Catch block of the error library. Allows for an error_variable to be set
 * to the current error which occurred.
 * 
 * @param error_variable A variable name or declaration which will be set to the
 * current error. Note that the scope of declarations in @p error_variable is
 * limited to the ``error_catch`` block.
 */
#define error_catch(error_variable) \
        error_variable = __em_ptr->current_error; \
        memcpy(__em_ptr, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
        if(__setjmp_result != 0)

/**
 * @brief Catch block of the error library. Same as @ref error_catch, but this does not
 * require an error variable. This can be used when you don't care which error whas thrown,
 * but only that an error has been thrown.
 */
#define error_catch_any \
        memcpy(__em_ptr, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
        if(__setjmp_result != 0)

/**
 * @brief Ends the error handling block.
 */
#define error_end_handling \
    }