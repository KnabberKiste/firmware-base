#include <knabberkiste/util/error.h>

volatile __error_manager_state_t __error_manager_state;

void error_handler(error_t* error) {

}

#if __has_include("FreeRTOS.h")

    void error_throw(error_code_t error_code, const char* error_message) {
        __error_manager_state_t* __em_ptr;

        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            /* FreeRTOS scheduler is running */
            __em_ptr = pvTaskGetThreadLocalStoragePointer(NULL, __THREAD_LOCAL_ERROR_MANAGER_STATE_INDEX);
        } else { \
            /* FreeRTOS scheduler is not running */
            __em_ptr = &__error_manager_state;
        }

        __em_ptr->current_error.error_code = error_code;
        __em_ptr->current_error.error_message = error_message;
        
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

void error_throw(error_code_t error_code, const char* error_message) {    
    __error_manager_state.current_error.error_code = error_code;
    __error_manager_state.current_error.error_message = error_message;
    
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