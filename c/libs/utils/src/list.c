#include "utils/list.h"
#include "utils/copy-interface.h"

#include <stdlib.h>

struct List {
    CopyInterface copy_ops;

    void **elements;

    size_t len;

    size_t alloc;
};

#define LIST_INIT_ALLOC 8

List *new_list(const struct CopyInterface *copy_ops) {
    List *list = malloc(sizeof(List));
    list->copy_ops = *copy_ops;
    list->elements = malloc(sizeof(void *) * LIST_INIT_ALLOC);
    list->len = 0;
    list->alloc = LIST_INIT_ALLOC;
    return list;
}

List *copy_list(const List *list) {
    List *copy = malloc(sizeof(List));
    copy->elements = malloc(sizeof(void *) * list->alloc);
    copy->copy_ops = list->copy_ops;
    copy->len = list->len;
    copy->alloc = list->alloc;
    for (size_t i = 0; i < list->len; i++) {
        copy->elements[i] = list->copy_ops.copy_val(list->elements[i]);
    }
    return copy;
}

void free_list(List *list) {
    for (size_t i = 0; i < list->len; i++) {
        list->copy_ops.free_val(list->elements[i]);
    }
    free(list->elements);
    free(list);
}

static void list_reserve_space(List *list, size_t len) {
    if (list->alloc < len) {
        do {
            list->alloc *= 2;
        } while (list->alloc < len);

        list->elements = realloc(list->elements, sizeof(void *) * list->alloc);
    }
}

void list_add(List *list, const void *val) {
    list_reserve_space(list, list->len + 1);
    list->elements[list->len] = list->copy_ops.copy_val(val);
    list->len++;
}

void *list_pop(List *list) {
    list->len--;
    return list->elements[list->len];
}

size_t list_len(const List *list) {
    return list->len;
}

bool list_has(const List *list, const void *val, bool (*cmp)(const void *cmp_val,
                                                             const void *list_val))
{
    for (size_t i = 0; i < list->len; i++) {
        if (cmp(val, list->elements)) {
            return true;
        }
    }
    return false;
}

bool list_empty(const List *list) {
    return list->len == 0;
}

const void *list_get(const List *list, size_t index) {
    return list->elements[index];
}

void *list_get_mutable(List *list, size_t index) {
    return list->elements[index];
}

// "Generic" versions of some functions that are used to satisfy interfaces

static void *copy_list_generic(const void *list) {
    return copy_list(list);
}

static void free_list_generic(void *list) {
    free_list(list);
}

const CopyInterface *LIST_COPY_OPS = &(CopyInterface) {
    copy_list_generic,
    free_list_generic,
};
