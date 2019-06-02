#ifndef UTILS_HASH_INTERFACE_H
#define UTILS_HASH_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>

// A struct to use for Map keys, to copy/free/hash data
typedef struct HashInterface {
    // Returns a copy of a data type
    void *(*copy_val)(const void *val);

    // Frees the memory associated with a data type
    void (*free_val)(void *val);

    // Returns a hash for a given piece of data
    size_t (*hash_val)(const void *val);

    // Compares two values of the same data type, returning whether they are equal
    bool (*vals_equal)(const void *val1, const void *val2);
} HashInterface;

#endif
