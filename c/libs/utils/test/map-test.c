#include "test/test.h"
#include "utils/copy-interface.h"
#include "utils/hash-interface.h"
#include "utils/map.h"
#include "utils/list.h"

#include <stdlib.h>

void *copy_int(const void *val) {
    int *copy = malloc(sizeof(int));
    *copy = * (int *) val;
    return copy;
}

void free_int(void *val) {
    free(val);
}

size_t hash_int(const void *val) {
    return *(int *) val;
}

bool ints_equal(const void *val1, const void *val2) {
    return *(int *) val1 == *(int *) val2;
}

const CopyInterface *INT_COPY_OPS = &(CopyInterface) {
    copy_int, free_int,
};

const HashInterface *INT_HASH_OPS = &(HashInterface) {
    copy_int, free_int, hash_int, ints_equal,
};

int main() {
    Map *map = new_map(INT_HASH_OPS, INT_COPY_OPS);

    ASSERT_EQUAL(map_size(map), 0);

    int one = 1;
    map_set(map, &one, &one);

    ASSERT_EQUAL(map_size(map), 1);
    ASSERT_TRUE(map_has(map, &one));
    ASSERT_EQUAL(* (int *) map_get(map, &one), 1);

    int two = 2;
    ASSERT_FALSE(map_has(map, &two));

    for (size_t i = 1; i <= 100; i++) {
        int key = i;
        int doubled = i * 2;
        map_set(map, &key, &doubled);
        ASSERT_EQUAL(map_size(map), i);
    }

    for (int i = 1; i <= 100; i++) {
        ASSERT_TRUE(map_has(map, &i));
        ASSERT_EQUAL(* (int *) map_get(map, &i), i * 2);
    }

    Map *copy = copy_map(map);
    ASSERT_EQUAL(map_size(copy), 100);
    
    for (int i = 1; i <= 100; i++) {
        ASSERT_TRUE(map_has(copy, &i));
        ASSERT_EQUAL(* (int *) map_get(copy, &i), i * 2);
    }

    return test_status();
}
