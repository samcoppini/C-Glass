#ifndef UTILS_MAP_H
#define UTILS_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Map Map;
struct CopyInterface;
struct HashInterface;
struct List;

// Returns a new map that uses the given interfaces
Map *new_map(const struct HashInterface *key_ops,
             const struct CopyInterface *val_ops);

// Returns a copy of a map
Map *copy_map(const Map *map);

// Frees the memory associated with a map
void free_map(Map *map);

// Sets a value in the map
void map_set(Map *map, const void *key, const void *val);

// Returns whether the map has a given key
bool map_has(const Map *map, const void *key);

// Returns a list of the map's keys. Must be freed by the user
struct List *map_get_keys(const Map *map);

// Returns the value associated with a key in the map
const void *map_get(const Map *map, const void *key);

// Returns a modifiable value associated with a key in the map
void *map_get_mutable(Map *map, const void *key);

// Returns how many key/value pairs are in the map
size_t map_size(const Map *map);

#endif
