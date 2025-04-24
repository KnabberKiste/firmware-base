#pragma once

#include <knabberkiste/io.h>
#include <knabberkiste/util/macro_util.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t error_code_t;

typedef struct {
    error_code_t error_code;
    const char* error_message;
} error_t;

typedef struct {
    jmp_buf try_buf;
    bool try_active;
    error_t current_error;
} __error_manager_state_t;

extern volatile __error_manager_state_t __error_manager_state;

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
        

    #define error_catch(error_variable) \
            error_variable = __em_ptr->current_error; \
            memcpy(__em_ptr, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
            if(__setjmp_result != 0)

    #define error_catch_any \
            memcpy(__em_ptr, &__previous_error_manager_state, sizeof(__previous_error_manager_state)); \
            if(__setjmp_result != 0)

    #define error_end_handling \
        }