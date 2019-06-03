#ifndef UTILS_SET_H
#define UTILS_SET_H

typedef struct Set Set;
struct HashInterface;

Set *new_set(const struct HashInterface *hash_ops);

void free_set(Set *set);

bool set_has(const Set *set, const void *val);

void set_add(const Set *set, const void *val);

#endif
