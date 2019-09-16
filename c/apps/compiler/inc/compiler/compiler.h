#ifndef COMPILER_COMPILER_H
#define COMPILER_COMPILER_H

struct Map;
struct String;

struct String *compile_classes(const struct Map *classes);

#endif
