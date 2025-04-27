#include <knabberkiste/util/error.h>
#include <knabberkiste/hal/vcp_debug.h>
#include <knabberkiste/io.h>
#include <stdio.h>

volatile __error_manager_state_t __error_manager_state;
volatile __error_manager_state_t __previous_error_manager_state_global;
volatile __error_manager_state_t* volatile __em_ptr_global;
volatile jmp_buf __yield_buf;

#if __has_include("FreeRTOS.h")
    #include <FreeRTOS.h>
    #include <task.h>
    #include <portable.h>
#endif

void __attribute__((weak)) uncaught_error_handler(error_t* error) {
    vcp_print("\r\n\r\nUncaught error ");
    vcp_print(error->error_name);
    vcp_print(": ");
    vcp_println(error->error_message);
    vcp_print("\tFile: ");
    vcp_println(error->origin_file);
    vcp_print("\tFunction: ");
    vcp_println(error->origin_function);
    vcp_print("\tContext: ");

    if(CORTEX_ACTIVE_INTERRUPT_VECTOR) {
        /* Error was thrown in ISR */
        vcp_println("<ISR> [MCU will be blocked]");
    }
    #if __has_include("FreeRTOS.h")
        else if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            /* FreeRTOS scheduler is running */
            vcp_print("FreeRTOS task '");
            vcp_print(pcTaskGetName(0));
            vcp_println("' [will be deleted]");
        }
    #endif
    else { \
        /* FreeRTOS scheduler is not running */
        vcp_println("<no task> [MCU will be blocked]");
    }
}

#if __has_include("FreeRTOS.h")
    void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, const char* origin_function) {
        volatile __error_manager_state_t* __em_ptr;

        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED && !CORTEX_ACTIVE_INTERRUPT_VECTOR) {
            /* FreeRTOS scheduler is running */
            __em_ptr = pvTaskGetThreadLocalStoragePointer(NULL, __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX);
        } else { \
            /* FreeRTOS scheduler is not running */
            __em_ptr = &__error_manager_state;
        }
        
        if(__em_ptr != 0 && __em_ptr->try_active) {
            /* Current code is wrapped in a try/catch block. */
            __em_ptr->error_occurred = true;

            __em_ptr->current_error.error_code = error_code;
            __em_ptr->current_error.error_message = error_message;
            __em_ptr->current_error.error_name = error_name;
            __em_ptr->current_error.origin_file = origin_file;
            __em_ptr->current_error.origin_function = origin_function;

            // Jump to the catch block
            longjmp((int*)&(__em_ptr->try_buf), 1);
        }

        error_t current_error;

        current_error.error_code = error_code;
        current_error.error_message = error_message;
        current_error.error_name = error_name;
        current_error.origin_file = origin_file;
        current_error.origin_function = origin_function;

        /* Current code is not wrapped in a try/catch block. */
        uncaught_error_handler(&current_error);
            
        if(CORTEX_ACTIVE_INTERRUPT_VECTOR) {
            while(1);
        } else if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            __enable_irq();
            vTaskDelete(NULL);
            while(1);
        } else {
            while(1);
        }
    }

#else

    void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, const char* origin_function) {    
        __error_manager_state.error_occurred = true;
        __error_manager_state.current_error.error_code = error_code;
        __error_manager_state.current_error.error_name = error_name;
        __error_manager_state.current_error.error_message = error_message;
        __error_manager_state.current_error.origin_file = origin_file;
        __error_manager_state.current_error.origin_function = origin_function;
        
        if(__error_manager_state.try_active) {
            /* Current code is wrapped in a try/catch block. */
            // Jump to the catch block
            longjmp(&(__error_manager_state.try_buf), 1);
        }

        /* Current code is not wrapped in a try/catch block. */
        uncaught_error_handler(&(__error_manager_state.current_error));
            
        while(1); // Stall if error handler returns (it shouldn't)
    }

#endif