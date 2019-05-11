#ifndef UTILS_MAP_H
#define UTILS_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Map Map;
struct CopyInterface;
struct HashInterface;

Map *new_map(const struct HashInterface *key_ops,
             const struct CopyInterface *val_ops);

Map *copy_map(const Map *map);

void free_map(Map *map);

void map_set(Map *map, const void *key, const void *val);

bool map_has(const Map *map, const void *key);

const void *map_get(const Map *map, const void *key);

void *map_get_mutable(Map *map, const void *key);

size_t map_size(const Map *map);

#endif
