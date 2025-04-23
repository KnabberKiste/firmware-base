#include <stdlib.h>
#include <knabberkiste/util/fifo.h>
#include <string.h>

void _fifo_put(volatile _FIFO_t* fifo, void* element) {
    // Check if the FIFO is already full
    if(fifo->_count >= fifo->_fifo_size) {
        // TODO: FIFO ERROR FULL
        return;
    } else {
        // Increment the end pointer of the FIFO, allowing it to overflow
        // to the start of the buffer
        fifo->_end = (fifo->_end + 1) % fifo->_fifo_size;
        volatile void* end_ptr = fifo->_buf + fifo->_element_size * fifo->_end;

        memcpy((void*)end_ptr, element, fifo->_element_size);
        fifo->_count++;
    }
}

void _fifo_get(volatile _FIFO_t* fifo, void* element) {
    if(fifo->_count == 0) {
        // TODO: FIFO ERROR EMPTY
        return;
    } else {
        volatile void* ptr = _fifo_get_direct(fifo);
        memcpy(element, (void*)ptr, fifo->_element_size);
    }
}

volatile void* _fifo_get_direct(volatile _FIFO_t* fifo) {
    if(fifo->_count == 0) {
        // TODO: FIFO ERROR EMPTY
        return;
    } else {
        // Increment the start pointer of the FIFO, allowing it to overflow
        // to the start of the buffer
        fifo->_start = (fifo->_start + 1) % fifo->_fifo_size;
        volatile void* start_ptr = fifo->_buf + fifo->_element_size * fifo->_start;

        fifo->_count--;
        return start_ptr;
    }
}