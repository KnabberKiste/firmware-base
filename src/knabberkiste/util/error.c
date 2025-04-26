#include <knabberkiste/util/error.h>

volatile __error_manager_state_t __error_manager_state;
volatile __error_manager_state_t __previous_error_manager_state_global;
volatile __error_manager_state_t* volatile __em_ptr_global;

void error_handler(error_t* error) {

}

#if __has_include("FreeRTOS.h")

    void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, unsigned int origin_line) {
        __error_manager_state_t* __em_ptr;

        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            /* FreeRTOS scheduler is running */
            __em_ptr = pvTaskGetThreadLocalStoragePointer(NULL, __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX);
        } else { \
            /* FreeRTOS scheduler is not running */
            __em_ptr = &__error_manager_state;
        }

        __em_ptr->error_occurred = true;

        __em_ptr->current_error.error_code = error_code;
        __em_ptr->current_error.error_message = error_message;
        __em_ptr->current_error.error_name = error_name;
        __em_ptr->current_error.origin_file = origin_file;
        __em_ptr->current_error.origin_line = origin_line;
        
        if(__em_ptr != 0 && __em_ptr->try_active) {
            /* Current code is wrapped in a try/catch block. */
            // Jump to the catch block
            longjmp(&(__em_ptr->try_buf), 1);
        }

        /* Current code is not wrapped in a try/catch block. */
        error_handler(&(__em_ptr->current_error));
            
        while(1); // Stall if error handler returns (it shouldn't)
    }

#else

    void _error_throw(error_code_t error_code, const char* error_name, const char* error_message, const char* origin_file, unsigned int origin_line) {    
        __error_manager_state.error_occurred = true;
        __error_manager_state.current_error.error_code = error_code;
        __error_manager_state.current_error.error_name = error_name;
        __error_manager_state.current_error.error_message = error_message;
        __error_manager_state.current_error.origin_file = origin_file;
        __error_manager_state.current_error.origin_line = origin_line;
        
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