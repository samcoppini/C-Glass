#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <stdbool.h>

struct List;
struct Map;
struct Stream;

struct Map *parse_classes(struct Stream *stream);

struct Map *classes_from_files(struct List *filenames, bool include_builtins);

#endif
