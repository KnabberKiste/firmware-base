#include <knabberkiste/util/varbuf.h>
#include <knabberkiste/util/error.h>
#include <stdint.h>

static struct _varbuf {
    size_t length;
    char elements[0];
};

static struct _varbuf* _varbuf_get(void** buf) {
    if(*buf == 0) {
        error_throw(ERR_ALLOCATION, "varbuf is empty.");
    }
    return *buf - offsetof(struct _varbuf, elements);
}

size_t _varbuf_get_length(void** buf) {
    if(*buf == 0) return 0;

    struct _varbuf* varbuf = _varbuf_get(buf);
    return varbuf->length;
}

void _varbuf_push_chunk(void** buf, void* element, size_t element_size, size_t number_of_elements) {
    critical_block {
        if(*buf == 0) {
            struct _varbuf* varbuf = malloc(sizeof(struct _varbuf));
            if(varbuf == 0) error_throw(ERR_ALLOCATION, "Initial varbuf allocation failed.");
            *buf = &(varbuf->elements);
            varbuf->length = 0;
        }
        struct _varbuf* varbuf = _varbuf_get(buf);
        
        // Resize the buffer to fit the new number of elements
        size_t previous_size = varbuf->length * element_size + sizeof(struct _varbuf);
        size_t new_size = previous_size + number_of_elements * element_size;
        varbuf = realloc(varbuf, new_size);

        // Check if the realloaction has succeeded
        if(varbuf == 0) error_throw(ERR_ALLOCATION, "varbuf reallocation failed.");

        // Reassign the user pointer
        *buf = &(varbuf->elements);

        // Copy the elements to the buffer
        memcpy(&(varbuf->elements[varbuf->length * element_size]), element, number_of_elements * element_size);

        // Increase the varbuf's length
        varbuf->length += number_of_elements;
    }
}

void _varbuf_pop_chunk(void** buf, void* dest, size_t start_idx, size_t elements, size_t element_size) {
    critical_block {
        struct _varbuf* varbuf = _varbuf_get(buf);

        // Check if the index is in range
        if(
            *buf == 0 ||
            varbuf->length < (start_idx + elements)
        ) {
            error_throw(ERR_RANGE, "varbuf index out of range.");
        }

        // Copy the elements to the destination
        memcpy(dest, &(varbuf->elements[start_idx * element_size]), elements * element_size);

        // Move the elements after the popped elements to the front
        size_t remaining_elements = varbuf->length - (start_idx + elements);
        if(remaining_elements > 0) {
            // Move the elements after the popped elements to the front
            memmove(&(varbuf->elements[start_idx * element_size]), &(varbuf->elements[element_size * (start_idx + elements)]), remaining_elements * element_size);
        }

        // Resize the buffer
        varbuf->length -= elements;
        if(varbuf->length > 0) {
            size_t new_size = varbuf->length * element_size + sizeof(struct _varbuf);
            varbuf = realloc(varbuf, new_size);

            // Check if the realloaction has succeeded
            if(varbuf == 0 && varbuf->length > 0) error_throw(ERR_ALLOCATION, "varbuf reallocation failed.");
        } else {
            varbuf_clear(*buf);
        }
    }
}

void _varbuf_clear(void** buf) {
    critical_block {
        if(*buf != 0) {
            struct _varbuf* varbuf = _varbuf_get(buf);
            free(varbuf);
            *buf = 0;
        }
    }
}