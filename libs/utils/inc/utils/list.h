#ifndef UTILS_LIST_H
#define UTILS_LIST_H

#include <stddef.h>

typedef struct List List;
struct CopyInterface;

const struct CopyInterface *LIST_COPY_OPS;

List *new_list(const struct CopyInterface *copy_ops);

List *copy_list(const List *list);

void free_list(List *list);

void list_add(List *list, const void *val);

size_t list_len(const List *list);

const void *list_get(const List *list, size_t index);

void *list_get_mutable(List *list, size_t index);

#endif
