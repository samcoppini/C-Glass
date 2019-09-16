#ifndef UTILS_SET_H
#define UTILS_SET_H

#include <stdbool.h>

typedef struct Set Set;
struct HashInterface;
struct List;

Set *new_set(const struct HashInterface *hash_ops);

struct List *set_to_list(const Set *set);

void free_set(Set *set);

bool set_has(const Set *set, const void *val);

void set_add(const Set *set, const void *val);

#endif
