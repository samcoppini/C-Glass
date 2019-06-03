#include "utils/set.h"
#include "utils/copy-interface.h"
#include "utils/map.h"

Set *new_set(const struct HashInterface *hash_ops) {
    Set *set = (Set *) new_map(hash_ops, INT_COPY_OPS);
    return set;
}

void free_set(Set *set) {
    free_map((Map *) set);
}

bool set_has(const Set *set, const void *val) {
    return map_has((Map *) set, val);
}

void set_add(const Set *set, const void *val) {
    int dummy = 1;

    map_set((Map *) set, val, &dummy);
}
