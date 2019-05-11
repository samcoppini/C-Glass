#ifndef UTILS_HASH_INTERFACE_H
#define UTILS_HASH_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct HashInterface {
    void *(*copy_val)(const void *val);

    void (*free_val)(void *val);

    size_t (*hash_val)(const void *val);

    bool (*vals_equal)(const void *val1, const void *val2);
} HashInterface;

#endif
