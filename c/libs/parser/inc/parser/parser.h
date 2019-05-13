#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <stdbool.h>

struct Map;
struct Stream;

struct Map *parse_classes(struct Stream *stream);

#endif
