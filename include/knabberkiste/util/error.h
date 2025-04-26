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
 * }
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
    /// @brief Name of the error code in the code.
    const char* error_name;
    /// @brief String-type error message passed with the error code.
    const char* error_message;
    /// @brief String-type name of the file from which the error originates.
    const char* origin_file;
    /// @brief String-type name of the function from which the error originates.
    const char* origin_function;
} error_t;

/**
 * @internal
 * @brief Struct representing the state of the error manager for
 * any given task.
 */
typedef struct {
    /// @brief jmp_buf for ``longjmp`` to jump to in case of an error.
    jmp_buf try_buf;
    /// @brief Whether the code is currently wrapped in a ``try`` block.
    bool try_active;
    /// @brief Current error state which was thrown.
    error_t current_error;
    /// @brief Whether an error has occurred.
    bool error_occurred;
} __error_manager_state_t;

/**
 * @internal
 * @brief Error manager state of the default task.
 */
extern volatile __error_manager_state_t __error_manager_state;

/**
 * @internal
 * @brief Error manager state of the default task.
 */
extern volatile __error_manager_state_t __previous_error_manager_state_global;

/**
 * @internal
 * @brief Internal pointer to the current error manager state.
 */
extern volatile __error_manager_state_t* volatile __em_ptr_global;

/**
 * @internal
 * @brief Internal error throw function.
 * 
 * @param error_code Error code identifying the error.
 * @param error_name Name of the error code.
 * @param error_message Error message passed along with the error.
 * @param origin_file File from which the error originates.
 * @param origin_function Function from which the error originates.
 */
void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, const char* origin_function);

/**
 * @brief Throws an error with the given error information.
 * 
 * @param error_code Error code to throw.
 * @param error_message Error message to pass along with the error code.
 */
#define error_throw(error_code, error_message) _error_throw(error_code, #error_code, error_message, __FILE__, __FUNCTION__)

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
                __em_ptr->error_occurred = false; \
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

#define __close_try \
        __disable_irq(); \
        __em_ptr_global = __em_ptr; \
        memcpy(&__previous_error_manager_state_global, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
    }

/**
 * @brief Catch block of the error library. Allows for an error_variable to be set
 * to the current error which occurred.
 * 
 * @param error_variable A variable name or declaration which will be set to the
 * current error. Note that the scope of declarations in @p error_variable is
 * limited to the ``error_catch`` block.
 */
#define error_catch(error_variable) \
    __close_try \
    error_variable = __em_ptr_global->current_error; \
    for( \
        error_variable = __em_ptr_global->current_error; \
        __em_ptr_global->error_occurred; \
        __em_ptr_global->error_occurred, \
        memcpy(__em_ptr_global, &__previous_error_manager_state_global, sizeof(__previous_error_manager_state_global)), __enable_irq() \
    )

/**
 * @brief Catch block of the error library. Same as @ref error_catch, but this does not
 * require an error variable. This can be used when you don't care which error whas thrown,
 * but only that an error has been thrown.
 */
#define error_catch_any \
        memcpy(__em_ptr, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
        if(__setjmp_result != 0)