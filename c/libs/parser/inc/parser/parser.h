#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <stdbool.h>

struct GlassProgramBuilder;
struct List;
struct Map;
struct Stream;

bool parse_classes(struct GlassProgramBuilder *builder,
                   struct Stream *stream);

struct Map *classes_from_files(struct List *filenames,
                               bool include_builtins,
                               bool resolve_inheritance);

#endif
