/**
 * @file fifo.h
 * @author Gabriel Heinzer
 * @brief Generic, thread-safe FIFO queuing library.
 */

#pragma once

#include <stdlib.h>
#include <knabberkiste/util/macro_util.h>

/**
 * @internal
 * @brief Internal FIFO structure. Do not use directly.
 */
typedef struct {
    size_t _start;
    size_t _end;
    size_t _count;
    const size_t _element_size;
    const size_t _fifo_size;
    volatile void* _buf;
} _FIFO_t;

/**
 * @brief Opaque type for a FIFO-type queue.
 * 
 */
typedef _FIFO_t FIFO_t;

/**
 * @internal
 * @brief Internal function which puts the @p element into the @p fifo.
 * 
 * @param fifo FIFO in which the element will be inserted.
 * @param element Pointer to the element to insert.
 */
void _fifo_put(volatile _FIFO_t* fifo, void* element);
/**
 * @internal
 * @brief Internal function which gets the @p element from the @p fifo.
 * 
 * @param fifo FIFO from which to get the element.
 * @param element Pointer to the destination where the element
 * will be copied.
 */
void _fifo_get(volatile _FIFO_t* fifo, void* element);
/**
 * @internal
 * @brief Internal function which gets the pointer to the @p element from the @p fifo.
 * 
 * @param fifo FIFO from which to get the element.
 * @return Pointer to the element.
 */
volatile void* _fifo_get_direct(volatile _FIFO_t* fifo);

/**
 * @brief Declares a new FIFO with the given @p type, @p name and @p size. You can
 * specify additional qualifiers for the FIFO variable, e.g. static.
 * 
 * @code{.c}
 * // At the desired scope of your FIFO, do:
 * fifo_declare(type, name, fifo_size);
 * @endcode
 * 
 * This can also be used in local scope, i.e. allocated on stack.
 * 
 * @param type Element type which the FIFO will contain.
 * @param name Name under which the FIFO can be accessed.
 * @param fifo_size Size of the FIFO.
 * @param qualifiers Additional qualifiers which will be placed before the
 * type of the FIFO.
 */
#define fifo_declare_qualifier(type, name, fifo_size, qualifiers) \
    qualifiers volatile type TOKEN_CONCAT(__INTERNAL_FIFOBUF_, __LINE__)[fifo_size]; \
    typedef type TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, name); \
    qualifiers volatile _FIFO_t name = { \
        ._start = 0, \
        ._end = 0, \
        ._count = 0, \
        ._element_size = sizeof(type), \
        ._fifo_size = fifo_size, \
        ._buf = TOKEN_CONCAT(__INTERNAL_FIFOBUF_, __LINE__) \
    }

/**
 * @brief Declares a new FIFO with the given @p type, @p name and @p size.
 * 
 * @code{.c}
 * // At the desired scope of your FIFO, do:
 * fifo_declare(type, name, fifo_size);
 * @endcode
 * 
 * This can also be used in local scope, i.e. allocated on stack.
 * 
 * @param type Element type which the FIFO will contain.
 * @param name Name under which the FIFO can be accessed.
 * @param fifo_size Size of the FIFO.
 */
#define fifo_declare(type, name, fifo_size) fifo_declare_qualifier(type, name, fifo_size,)

/**
 * @brief Puts the given @p element to the end of the @p fifo.
 * 
 * If the FIFO is already full, this blocks until space is available.
 * 
 * @param fifo FIFO to which the element should be appended.
 * @param element Element which will be appended to the FIFO.
 */
#define fifo_put(fifo, element) _fifo_put((_FIFO_t*)(&(fifo)), (void*)(&(element)))

/**
 * @brief Puts the given @p element, which may be a lliteral, to the end of the @p fifo.
 * 
 * If the FIFO is already full, this blocks until space is available.
 * 
 * @param fifo FIFO to which the element should be appended.
 * @param element Element which will be appended to the FIFO.
 */
#define fifo_put_literal(fifo, element) { \
        TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, fifo) TOKEN_CONCAT(__INTERNAL_FIFO_TEMP_, fifo) = element; \
        fifo_put(fifo, TOKEN_CONCAT(__INTERNAL_FIFO_TEMP_, fifo)); \
    }

/**
 * @brief Gets an element from the @p fifo and writes it ot the variable @p element.
 * 
 * If the FIFO is already empty, this blocks until an element becomes available.
 * 
 * @param fifo FIFO from which to get the element.
 * @param element Variable to which the element will be written to.
 */
#define fifo_get(fifo, element) _fifo_get((_FIFO_t*)(&(fifo)), (void*)(&(element)))

/**
 * @brief Gets an element from the @p fifo and returns it. Of course, you can only
 * use this macro, if you can assign the type directly. If not, use @ref fifo_get().
 * 
 * @param fifo FIFO from which to get the element.
 * 
 * @returns Returns the value of the element.
 */
#define fifo_get_direct(fifo) *(TOKEN_CONCAT(__INTERNAL_FIFO_CLIENT_TYPE_, fifo)*)(_fifo_get_direct((_FIFO_t*)(&(fifo))))

/**
 * @brief Gets the size of the queue, i.e. the max. number of elements.
 * 
 * @param fifo The FIFO queue you want to access.
 */
#define fifo_get_size(fifo) ((fifo)._fifo_size)
/**
 * @brief Gets the element size of the queue, i.e. the size of one element in bytes.
 * 
 * @param fifo The FIFO queue you want to access.
 */
#define fifo_get_element_size(fifo) ((fifo)._element_size)
/**
 * @brief Gets the number of elements currently in the queue.
 * 
 * @param fifo The FIFO queue you want to access.
 */
#define fifo_get_element_count(fifo) ((fifo)._count)
/**
 * @brief Checks if the given FIFO queue is empty, i.e. none of the slots in the queue are occupied.
 * 
 * @param fifo The FIFO queue you want to access.
 */
#define fifo_empty(fifo) ((fifo)._count == 0)
/**
 * @brief Checks if the given FIFO queue is full, i.e. all the slots in the queue are occupied.
 * 
 * @param fifo The FIFO queue you want to access.
 */
#define fifo_full(fifo) ((fifo)._count == (fifo)._fifo_size)