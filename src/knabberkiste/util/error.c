#include <knabberkiste/util/error.h>
#include <knabberkiste/hal/vcp_debug.h>
#include <stdio.h>

volatile __error_manager_state_t __error_manager_state;
volatile __error_manager_state_t __previous_error_manager_state_global;
volatile __error_manager_state_t* volatile __em_ptr_global;

#define ERROR_INFO_SIZE 128

#if __has_include("FreeRTOS.h")
    static void error_handler(error_t* error) {
        static char error_info[ERROR_INFO_SIZE];
        snprintf(error_info, ERROR_INFO_SIZE, "%s [file '%s', function '%s']", error->error_name, error->origin_file, error->origin_function);
        vcp_print("Uncaught error ");
        vcp_print(error_info);
        vcp_print(" in task ");

        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            /* FreeRTOS scheduler is running */
            vcp_println(pcTaskGetName(0));
        } else { \
            /* FreeRTOS scheduler is not running */
            vcp_println("<no task>");
        }

        vcp_print("\t");
        vcp_println(error->error_message);
        vcp_println("");

        while(1);
    }

    void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, const char* origin_function) {
        __error_manager_state_t* __em_ptr;

        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
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
            longjmp(&(__em_ptr->try_buf), 1);
        }

        error_t current_error;

        current_error.error_code = error_code;
        current_error.error_message = error_message;
        current_error.error_name = error_name;
        current_error.origin_file = origin_file;
        current_error.origin_function = origin_function;

        /* Current code is not wrapped in a try/catch block. */
        error_handler(&current_error);
            
        while(1); // Stall if error handler returns (it shouldn't)
    }

#else

    static void error_handler(error_t* error) {
        static char error_info[ERROR_INFO_SIZE];
        snprintf(error_info, ERROR_INFO_SIZE, "%s [file '%s', function '%s']", error->error_name, error->origin_file, error->origin_function);
        vcp_print("Uncaught error");
        vcp_println(error_info);
        vcp_print("\t");
        vcp_println(error->error_message);

        while(1);
    }

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
        error_handler(&(__error_manager_state.current_error));
            
        while(1); // Stall if error handler returns (it shouldn't)
    }

#endif