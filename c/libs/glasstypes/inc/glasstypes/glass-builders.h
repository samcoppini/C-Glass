#ifndef GLASSTYPES_GLASS_BUILDERS_H
#define GLASSTYPES_GLASS_BUILDERS_H

#include <stdbool.h>

typedef struct GlassClassBuilder GlassClassBuilder;
typedef struct GlassFuncBuilder GlassFuncBuilder;
struct GlassClass;
struct GlassCommand;
struct GlassFunction;
struct String;

GlassClassBuilder *new_class_builder(const struct String *name,
                                     const struct String *filename,
                                     unsigned line, unsigned col);

GlassFuncBuilder *new_func_builder(const struct String *name,
                                   const struct String *filename,
                                   unsigned line, unsigned col);

void free_class_builder(GlassClassBuilder *builder);

void free_func_builder(GlassFuncBuilder *builder);

struct GlassClass *build_glass_class(const GlassClassBuilder *builder);

struct GlassFunction *build_glass_function(const GlassFuncBuilder *builder);

bool builder_add_func(GlassClassBuilder *builder, const struct GlassFunction *func);

bool builder_add_command(GlassFuncBuilder *builder, const struct GlassCommand *cmd);

#endif
