/**
 * @file varbuf.h
 * @author Gabriel Heinzer
 * @brief Heap-allocated, variably-sized buffer, ideal for receiving data streams.
 * 
 * @section varbuf-usage Usage
 * 
 * You can declare a varbuf like this:
 * @code{.c}
 * char* mybuf;
 * @endcode
 * 
 * This is all you need for a varbuf of type `char`. You can also use any other type,
 * like `uint32_t` or a structure.
 * 
 * @warning As varbufs are allocated on the heap, you must remember to deallocate
 * them when your done. Use @ref varbuf_clear for that.
 * 
 * @subsection varbuf-accessing Accessing the elements
 * 
 * You can access the elements (including reading and modifying them) using square
 * braces:
 * 
 * @code{.c}
 * // Set an element
 * mybuf[5] = 'c';
 * 
 * // Read an element
 * printf(mybuf[7]);
 * @endcode
 * 
 * @section varbuf-internal Internal structure
 * The `varbuf` is allocated in heap at the position the pointer points to minus the size of
 * `size_t`. This is done so that the size of the buffer can be stored in heap as well.
 * 
 * `varbuf` can work with a pointer to any type and can therefore store a list of any arbitrary
 * type.
 */

#pragma once

#include <stdlib.h>

/* Internal function declarations */

size_t _varbuf_get_length(void** buf);
void _varbuf_push_chunk(void** buf, void* element, size_t element_size, size_t number_of_elements);

void _varbuf_pop_chunk(void** buf, void* dest, size_t start_idx, size_t elements, size_t element_size);

void _varbuf_clear(void** buf);

/**
 * @brief Gets the varbuf's element size.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * 
 * @returns The varbuf's element size.
 */
#define varbuf_element_size(buf) sizeof(*(buf))
/**
 * @brief Get's the length of the varbuf as in the number of elements it currently stores.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * 
 * @returns The varbuf's length as in the number of elements it currently stores.
 */
#define varbuf_length(buf) _varbuf_get_length((void*)&(buf))

/**
 * @brief Appends a single element to the buffer. Note that the element must be of the same
 * type as the buffer itself is.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param element The element to push to the buffer. This must not be a pointer, but the element
 * itself.
 * 
 * @returns Nothing.
 */
#define varbuf_push(buf, element) _varbuf_push_chunk((void*)&(buf), (void*)&(element), varbuf_element_size(buf), 1)

/**
 * @brief Same as @ref varbuf_push, but appends mulitple elements from an array at once.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param chunk_ptr A pointer to the first element of the array containing the elements
 * to push.
 * @param number_of_elements The number of elements contained in the array pointed to by
 * @p chunk_ptr.
 * 
 * @returns Nothing.
 */
#define varbuf_push_chunk(buf, chunk_ptr, number_of_elements) _varbuf_push_chunk((void*)&(buf), (void*)(chunk_ptr), varbuf_element_size(buf), number_of_elements)

/**
 * @brief Pops a single element from the end of the varbuf.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param dest_ptr A pointer to a destination variable in which the value will be stored.
 * 
 * @returns Nothing.
 */
#define varbuf_pop_end(buf, dest_ptr) _varbuf_pop_chunk((void*)&(buf), (void*)(dest_ptr), varbuf_length(buf) - 1, 1, varbuf_element_size(buf))
/**
 * @brief Pops a single element from the start of the varbuf.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param dest_ptr A pointer to a destination variable in which the value will be stored.
 * 
 * @returns Nothing.
 */
#define varbuf_pop_start(buf, dest_ptr) _varbuf_pop_chunk((void*)&(buf), (void*)(dest_ptr), 0, 1, varbuf_element_size(buf))

/**
 * @brief Pops a single element from the specified index of the varbuf.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param idx The index at which the element is currently located.
 * @param dest_ptr A pointer to a destination variable in which the value will be stored.
 * 
 * @returns Nothing.
 */
#define varbuf_pop_index(buf, idx, dest_ptr) _varbuf_pop_chunk((void*)&(buf), (void*)(dest_ptr), (idx), 1, varbuf_element_size(buf))

/**
 * @brief Pops multiple element from the specified index of the varbuf.
 * 
 * @param buf The varbuf to use for the length. This must not be a pointer to
 * the buffer, but just the buffer directly.
 * @param start_idx The index of the first element of the varbuf.
 * @param number_of_elements Number of elements to pop from the varbuf.
 * @param dest_ptr A pointer to a destination variable in which the value will be stored.
 * 
 * @returns Nothing.
 */
#define varbuf_pop_chunk(buf, start_idx, number_of_elements, dest_ptr) _varbuf_pop_chunk((void*)&(buf), (void*)(dest_ptr), (start_idx), (number_of_elements), varbuf_element_size(buf))

/**
 * @brief Clears a varbuf and deallocates it from the heap. After calling this,
 * it is safe to let the varbuf go out of scope without worrying about a memory
 * leak.
 */
#define varbuf_clear(buf) _varbuf_clear(&(buf))