#include <stdlib.h>
#include <knabberkiste/util/fifo.h>
#include <knabberkiste/util/error.h>
#include <knabberkiste/io.h>
#include <string.h>

void _fifo_put(volatile _FIFO_t* fifo, void* element) {
    while(fifo_full(*fifo));
    __disable_irq();
    // Increment the end pointer of the FIFO, allowing it to overflow
    // to the start of the buffer
    fifo->_end = (fifo->_end + 1) % fifo->_fifo_size;
    volatile void* end_ptr = fifo->_buf + fifo->_element_size * fifo->_end;

    memcpy((void*)end_ptr, element, fifo->_element_size);
    fifo->_count++;
    __enable_irq();
}

void _fifo_get(volatile _FIFO_t* fifo, void* element) {
    while(fifo_empty(*fifo));
    __disable_irq();
    volatile void* ptr = _fifo_get_direct(fifo);
    memcpy(element, (void*)ptr, fifo->_element_size);
    __enable_irq();
}

volatile void* _fifo_get_direct(volatile _FIFO_t* fifo) {
    if(fifo->_count == 0) {
        // TODO: FIFO ERROR EMPTY
        return;
    } else {
        __disable_irq();
        // Increment the start pointer of the FIFO, allowing it to overflow
        // to the start of the buffer
        fifo->_start = (fifo->_start + 1) % fifo->_fifo_size;
        volatile void* start_ptr = fifo->_buf + fifo->_element_size * fifo->_start;

        fifo->_count--;
        __enable_irq();
        return start_ptr;
    }
}