#ifndef UTILS_LIST_H
#define UTILS_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct List List;

struct CopyInterface;

const struct CopyInterface *LIST_COPY_OPS;

// Returns a pointer to a new list, which copies/frees data using the given
// copyinterface
List *new_list(const struct CopyInterface *copy_ops);

// Returns a copy of a given list
List *copy_list(const List *list);

// Frees the memory associated with a given list
void free_list(List *list);

// Copies an element to the end of a list
void list_add(List *list, const void *val);

// Removes the last element from a list, and returns it
void *list_pop(List *list);

// Returns the length of the list
size_t list_len(const List *list);

// Returns true if the list has an element that compares equal to the given
// element with the cmp function
bool list_has(const List *list, const void *val,
              bool (*cmp)(const void *cmp_val, const void *list_val));

// Returns whether the list is empty
bool list_empty(const List *list);

// Returns an element at a given index in the list
const void *list_get(const List *list, size_t index);

// Returns a mutable element at a given index in the list
void *list_get_mutable(List *list, size_t index);

#endif
