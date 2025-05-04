#include <stdlib.h>
#include <knabberkiste/util/fifo.h>
#include <knabberkiste/util/error.h>
#include <knabberkiste/io.h>
#include <string.h>

void _fifo_put(volatile _FIFO_t* fifo, void* element) {
    while(fifo_full(*fifo));
    critical_block {
        // Increment the end pointer of the FIFO, allowing it to overflow
        // to the start of the buffer
        fifo->_end = (fifo->_end + 1) % fifo->_fifo_size;
        volatile void* end_ptr = fifo->_buf + fifo->_element_size * fifo->_end;

        memcpy((void*)end_ptr, element, fifo->_element_size);
        fifo->_count++;
    }
}

void _fifo_get(volatile _FIFO_t* fifo, void* element) {
    while(fifo_empty(*fifo));
    critical_block {
        volatile void* ptr = _fifo_get_direct(fifo);
        memcpy(element, (void*)ptr, fifo->_element_size);
    }
}

volatile void* _fifo_get_direct(volatile _FIFO_t* fifo) {
    while(fifo_empty(*fifo));
    volatile void* start_ptr;
    critical_block {
        // Increment the start pointer of the FIFO, allowing it to overflow
        // to the start of the buffer
        fifo->_start = (fifo->_start + 1) % fifo->_fifo_size;
        start_ptr = fifo->_buf + fifo->_element_size * fifo->_start;

        fifo->_count--;
    }
    return start_ptr;
}