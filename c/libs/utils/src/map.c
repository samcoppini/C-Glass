#include "utils/map.h"
#include "utils/copy-interface.h"
#include "utils/hash-interface.h"
#include "utils/list.h"

#include <stdlib.h>

struct Map {
    HashInterface key_ops;

    CopyInterface val_ops;

    void **keys;

    void **vals;

    size_t used_slots;

    size_t alloc;
};

#define MAP_INIT_ALLOC   16
#define MAP_LOAD_FACTOR 0.5

Map *new_map(const HashInterface *key_ops, const CopyInterface *val_ops) {
    Map *map = malloc(sizeof(Map));
    map->key_ops = *key_ops;
    map->val_ops = *val_ops;
    map->keys = calloc(MAP_INIT_ALLOC, sizeof(void *));
    map->vals = malloc(sizeof(void *) * MAP_INIT_ALLOC);
    map->used_slots = 0;
    map->alloc = MAP_INIT_ALLOC;
    return map;
}

Map *copy_map(const Map *map) {
    Map *copy = malloc(sizeof(Map));
    copy->key_ops = map->key_ops;
    copy->val_ops = map->val_ops;
    copy->keys = calloc(map->alloc, sizeof(void *));
    copy->vals = malloc(sizeof(void *) * map->alloc);
    copy->used_slots = map->used_slots;
    copy->alloc = map->alloc;
    
    for (size_t i = 0; i < copy->alloc; i++) {
        if (map->keys[i] != NULL) {
            copy->keys[i] = map->key_ops.copy_val(map->keys[i]);
            copy->vals[i] = map->val_ops.copy_val(map->vals[i]);
        }
    }

    return copy;
}

void free_map(Map *map) {
    for (size_t i = 0; i < map->alloc; i++) {
        if (map->keys[i] != NULL) {
            map->key_ops.free_val(map->keys[i]);
            map->val_ops.free_val(map->vals[i]);
        }
    }
    free(map->keys);
    free(map->vals);
    free(map);
}

static size_t map_get_slot(const Map *map, const void *key) {
    size_t slot = map->key_ops.hash_val(key) & (map->alloc - 1);

    while (map->keys[slot] != NULL) {
        if (map->key_ops.vals_equal(map->keys[slot], key)) {
            return slot;
        }
        slot = (slot + 1) & (map->alloc - 1);
    }

    return slot;
}

static void map_resize(Map *map) {
    void **old_keys = map->keys;
    void **old_vals = map->vals;
    size_t old_size = map->alloc;

    map->alloc *= 2;
    map->keys = calloc(map->alloc, sizeof(void *));
    map->vals = malloc(sizeof(void *) * map->alloc);

    for (size_t i = 0; i < old_size; i++) {
        if (old_keys[i] != NULL) {
            size_t slot = map_get_slot(map, old_keys[i]);
            map->keys[slot] = old_keys[i];
            map->vals[slot] = old_vals[i];
        }
    }

    free(old_keys);
    free(old_vals);
}

void map_set(Map *map, const void *key, const void *val) {
    size_t slot = map_get_slot(map, key);

    if (map->keys[slot] != NULL) {
        map->val_ops.free_val(map->vals[slot]);
        map->vals[slot] = map->val_ops.copy_val(val);
    }
    else {
        map->keys[slot] = map->key_ops.copy_val(key);
        map->vals[slot] = map->val_ops.copy_val(val);
        map->used_slots++;

        if (map->used_slots >= map->alloc * MAP_LOAD_FACTOR) {
            map_resize(map);
        }
    }
}

bool map_has(const Map *map, const void *key) {
    size_t slot = map_get_slot(map, key);
    return map->keys[slot] != NULL;
}

List *map_get_keys(const Map *map) {
    CopyInterface key_copy_ops = {
        map->key_ops.copy_val,
        map->key_ops.free_val,
    };
    List *keys = new_list(&key_copy_ops);
    for (size_t i = 0; i < map->alloc; i++) {
        if (map->keys[i] != NULL) {
            list_add(keys, map->keys[i]);
        }
    }
    return keys;
}

const void *map_get(const Map *map, const void *key) {
    size_t slot = map_get_slot(map, key);
    if (map->keys[slot] == NULL) {
        return NULL;
    }
    return map->vals[slot];
}

void *map_get_mutable(Map *map, const void *key) {
    size_t slot = map_get_slot(map, key);
    return map->vals[slot];
}

size_t map_size(const Map *map) {
    return map->used_slots;
}
