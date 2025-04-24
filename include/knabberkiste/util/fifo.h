#pragma once

#include <stdlib.h>
#include <knabberkiste/util/macro_util.h>

typedef struct {
    size_t _start;
    size_t _end;
    size_t _count;
    const size_t _element_size;
    const size_t _fifo_size;
    volatile void* _buf;
} _FIFO_t;

void _fifo_put(volatile _FIFO_t* fifo, void* element);
void _fifo_get(volatile _FIFO_t* fifo, void* element);
volatile void* _fifo_get_direct(volatile _FIFO_t* fifo);

#define fifo_declare(type, name, fifo_size) \
    volatile type TOKEN_CONCAT(__INTERNAL_FIFOBUF_, __LINE__)[fifo_size]; \
    typedef type TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, name); \
    volatile _FIFO_t name = { \
        ._start = 0, \
        ._end = 0, \
        ._count = 0, \
        ._element_size = sizeof(type), \
        ._fifo_size = fifo_size, \
        ._buf = TOKEN_CONCAT(__INTERNAL_FIFOBUF_, __LINE__) \
    };

#define fifo_put(fifo, element) _fifo_put((_FIFO_t*)(&(fifo)), (void*)(&(element)))
#define fifo_put_literal(fifo, element) { \
        TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, fifo) TOKEN_CONCAT(__INTERNAL_FIFO_TEMP_, fifo) = element; \
        fifo_put(fifo, TOKEN_CONCAT(__INTERNAL_FIFO_TEMP_, fifo)); \
    }

#define fifo_get(fifo, element) _fifo_get((_FIFO_t*)(&(fifo)), (void*)(&(element)))
#define fifo_get_direct(fifo) *(TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, fifo)*)(_fifo_get_direct((_FIFO_t*)(&(fifo))))

#define fifo_get_size(fifo) (fifo._fifo_size)
#define fifo_get_element_size(fifo) (fifo._element_size)
#define fifo_get_element_count(fifo) (fifo._count)
#define fifo_empty(fifo) (fifo._count == 0)
#define fifo_full(fifo) (fifo._count == fifo._fifo_size)